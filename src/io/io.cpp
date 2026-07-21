#include "../helpers/errors/errors.hpp"
#include "io.hpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <assert.h>
#include "zlib.h"

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
  #include <fcntl.h>
  #include <io.h>
  #define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
  #define SET_BINARY_MODE(file)
#endif

namespace CustomIDE {

std::vector<uint8_t> IO::ReadBinaryFile(const std::string& filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    Errors::ExitWithError("Failed to open file", -5);
  }

  size_t fileSize = (size_t) file.tellg();
  std::vector<uint8_t> buffer(fileSize);

  file.seekg(0);
  file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

  file.close();

  return buffer;
}

std::string IO::ReadTextFile(const std::string& filename) {
  std::ifstream file(filename);

  if (!file.is_open()) {
    if (file.fail()) {
      std::cerr << "Error details: " << strerror(errno)
           << std::endl;
    }
    Errors::ExitWithError("Failed to open file", -5);
  }

  std::string fileContents;

  std::string line;
  while (getline(file, line)) {
    fileContents.append(line);
    fileContents.push_back('\n');
  }

  file.close();

  return fileContents;
}

void IO::WriteTextFile(const std::string& filename, const std::string& content) {
  std::ofstream file(filename);

  if (!file.is_open()) {
    if (file.fail()) {
      std::cerr << "Error details: " << strerror(errno)
           << std::endl;
    }
    Errors::ExitWithError("Failed to open file", -5);
  }

  file.write(content.c_str(), content.size());

  file.close();
}

bool IO::CompareByteValues(const std::vector<uint8_t>& Obj1, const uint8_t* Obj2, size_t BytesToCompare) {
  if (Obj1.size() < BytesToCompare) Errors::ExitWithError("Not enough bytes to compare", -7);

  for (size_t i = 0; i < BytesToCompare; i++) {
    if (static_cast<uint8_t>(Obj1[i]) != Obj2[i]) return false;
  }

  return true;
}

// TODO: Change to return an array of shared pointers (pointer to pointers)
// and have return type be the amount of images returned
bool IO::ReadImageFile(const std::string &filename, std::vector<std::shared_ptr<Image>>& OutImages) {
  std::vector<uint8_t> buffer = IO::ReadBinaryFile(filename);

  if (buffer.size() == 0) Errors::ExitWithError("File buffer size is 0", -6);

  if (CompareByteValues(buffer, PNG_MAGIC_NUMBERS, PNG_MAGIC_NUMBER_BYTE_AMOUNT)) {
    OutImages.push_back(IO::ParsePNGData(buffer, 0));
    return true;
  }

  if (CompareByteValues(buffer, BMP_MAGIC_NUMBERS, BMP_MAGIC_NUMBER_BYTE_AMOUNT)) {
    BITMAPFILEHEADER header = {
      .size = u32LE(buffer[2], buffer[3], buffer[4], buffer[5]),
      .reserved0 = u16LE(buffer[6], buffer[7]),
      .reserved1 = u16LE(buffer[8], buffer[9]),
      .offset = u32LE(buffer[10], buffer[11], buffer[12], buffer[13])
    };
    BITMAPINFOHEADER info = {
      .headerSize = u32LE(buffer[14], buffer[15], buffer[16], buffer[17]),
      .width = s32LE(buffer[18], buffer[19], buffer[20], buffer[21]),
      .height = s32LE(buffer[22], buffer[23], buffer[24], buffer[25]),
      .colourPlanes = u16LE(buffer[26], buffer[27]),
      .bitsPerPixel = u16LE(buffer[28], buffer[29]),
      .compressionMethod = u32LE(buffer[30], buffer[31], buffer[32], buffer[33]),
      .imageSize = u32LE(buffer[34], buffer[35], buffer[36], buffer[37]),
      .hResolution = s32LE(buffer[38], buffer[39], buffer[40], buffer[41]),
      .vResolution = s32LE(buffer[42], buffer[43], buffer[44], buffer[45]),
      .colours = u32LE(buffer[46], buffer[47], buffer[48], buffer[49]),
      .importantColours = u32LE(buffer[50], buffer[51], buffer[52], buffer[53])
    };
    OutImages.push_back(ParseBMPData(buffer, info, header.offset));
    return true;
  }

  if (CompareByteValues(buffer, ICO_MAGIC_NUMBERS, ICO_MAGIC_NUMBER_BYTE_AMOUNT)) {
    ParseICOData(buffer, OutImages);
    return true;
  }

  printf("[Warning]: File format not supported");
  return false;
}

std::shared_ptr<IO::Image> IO::ParsePNGData(const std::vector<uint8_t>& Data, uint32_t Offset) {
  using namespace IO;
  uint32_t offset = Offset + PNG_MAGIC_NUMBER_BYTE_AMOUNT;

  std::shared_ptr<Image> returnPtr = std::make_shared<Image>();

  PNGCHUNK chunk{};
  chunk.chunkSize = u32(Data[offset], Data[offset + 1], Data[offset + 2], Data[offset + 3]);
  chunk.chunkType = u32(Data[offset + 4], Data[offset + 5], Data[offset + 6], Data[offset + 7]);

  if (chunk.chunkType != PNG_IHDR_CHUNK_SIGNATURE) Errors::ExitWithError("No PNG image header found", -10);

  offset += sizeof(chunk.chunkSize) + sizeof(chunk.chunkType);

  PNGIMAGEHEADER imageHeader;
  imageHeader.width = u32(Data[offset], Data[offset + 1], Data[offset + 2], Data[offset + 3]);
  imageHeader.height = u32(Data[offset + 4], Data[offset + 5], Data[offset + 6], Data[offset + 7]);
  imageHeader.bitDepth = Data[offset + 8];
  imageHeader.colourType = Data[offset + 9];
  imageHeader.compressionMethod = Data[offset + 10];
  imageHeader.filterMethod = Data[offset + 11];
  imageHeader.interlaceMethod = Data[offset + 12];

  int BytesPerPixel = PNG::GetImageBitsPerPixel(imageHeader.bitDepth, (PNG::ColourType)imageHeader.colourType) / 8;
  size_t Stride = imageHeader.width * BytesPerPixel;
  size_t PixelDataSize = Stride * imageHeader.height;

  returnPtr->width = imageHeader.width;
  returnPtr->height = imageHeader.height;
  returnPtr->pixels = (uint8_t*)calloc(PixelDataSize, sizeof(uint8_t));

  offset += sizeof(PNGIMAGEHEADER) - sizeof(Pixel) + 1;
  chunk = {};
  std::vector<uint8_t> dataChunks;

  do {
    offset += sizeof(Pixel); // Add an extra 4 bytes to get past the CRC
    offset += chunk.chunkSize;
    chunk = {};
    chunk.chunkSize = u32(Data[offset], Data[offset + 1], Data[offset + 2], Data[offset + 3]);
    chunk.chunkType = u32(Data[offset + 4], Data[offset + 5], Data[offset + 6], Data[offset + 7]);
    offset += sizeof(Pixel) * 2; // Move offset to the byte after the size and type
    if (chunk.chunkType == PNG_IDAT_CHUNK_SIGNATURE) {
      dataChunks.reserve(dataChunks.size() + chunk.chunkSize);
      for (int i = 0; i < chunk.chunkSize; i++) {
        dataChunks.push_back(Data[offset + i]);
      }
    }
  } while (chunk.chunkType != PNG_IEND_CHUNK_SIGNATURE);

  std::vector<uint8_t> pixelVector;
  // IDAT chunks found
  // Need to decode the data into valid pixel data
  
  //int result = InflateDecoder(std::vector(Data.begin() + offset, Data.begin() + offset + chunk.chunkSize), pixelVector);
  int result = InflateDecoder(dataChunks, pixelVector);
  if (result != Z_OK)
    Errors::ExitWithError("Failed to decode PNG file", -24);

  // Then remove filtering

  size_t i = 0;
  size_t pixelArrayIndex = 0;
  for (size_t ScanLine = 0; ScanLine < imageHeader.height; ScanLine++) {
    uint8_t filterType = pixelVector[i];
    i++;
    for (size_t LineByteOffset = 0; LineByteOffset < Stride; LineByteOffset++) {
      uint8_t reconX = 0;
      uint8_t filterX = pixelVector[i];
      i++;
      if (filterType == 0)
        reconX = filterX;
      else if (filterType == 1)
        reconX = filterX + PNG::ReconA(ScanLine, LineByteOffset, Stride, BytesPerPixel, returnPtr->pixels);
      else if (filterType == 2)
        reconX = filterX + PNG::ReconB(ScanLine, LineByteOffset, Stride, returnPtr->pixels);
      else if (filterType == 3)
        reconX = filterX + PNG::ReconC(ScanLine, LineByteOffset, Stride, BytesPerPixel, returnPtr->pixels);
      else if (filterType == 4)
        reconX = filterX + PNG::PaethPredictor(PNG::ReconA(ScanLine, LineByteOffset, Stride, BytesPerPixel, returnPtr->pixels),
                                               PNG::ReconB(ScanLine, LineByteOffset, Stride, returnPtr->pixels),
                                               PNG::ReconC(ScanLine, LineByteOffset, Stride, BytesPerPixel, returnPtr->pixels));
      returnPtr->pixels[pixelArrayIndex++] = reconX;
    }
  }

  return returnPtr;
}

int IO::InflateDecoder(const std::vector<uint8_t>& Data, std::vector<uint8_t>& Output) {
  int ret;
  z_stream stream;
  uint32_t have;

  const long int CHUNK = 16384;
  size_t remainingData = Data.size();
  size_t chunkCount = 0;
  uint8_t in[CHUNK];
  uint8_t out[CHUNK];

  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;
  stream.avail_in = 0;
  stream.next_in = Z_NULL;

  ret = inflateInit(&stream);
  if (ret != Z_OK)
    return ret;

  do {
    size_t amountMoved;
    if (remainingData >= CHUNK) {
      amountMoved = CHUNK;
    } else {
      amountMoved = remainingData;
    }
    remainingData -= amountMoved;
    for (size_t i = 0; i < amountMoved; i++) {
      in[i] = Data[i + (CHUNK * chunkCount)];
    }
    stream.avail_in = amountMoved;
    if (stream.avail_in == 0)
      break;
    stream.next_in = in;

    do {
      stream.avail_out = CHUNK;
      stream.next_out = out;
      ret = inflate(&stream, Z_NO_FLUSH);
      assert(ret != Z_STREAM_ERROR);
      switch (ret) {
        case Z_NEED_DICT:
          ret = Z_DATA_ERROR;
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
          inflateEnd(&stream);
          return ret;
      }
      have = CHUNK - stream.avail_out;
      Output.reserve(Output.size() + have);
      for (size_t i = 0; i < have; i++) {
        Output.push_back(out[i]);
      }
    } while (stream.avail_out == 0);
    chunkCount++;
  } while (ret != Z_STREAM_END);

  inflateEnd(&stream);
  return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

constexpr int IO::PNG::GetImageBitsPerPixel(uint8_t BitsPerChannel, ColourType ColourMode) {
  switch (ColourMode) {
    case ColourType::Grayscale:
    case ColourType::Indexed:
      return 1 * BitsPerChannel;
    case ColourType::GrayscaleAlpha:
      return 2 * BitsPerChannel;
    case ColourType::Truecolour:
      return 3 * BitsPerChannel;
    case ColourType::TruecolourAlpha:
      return 4 * BitsPerChannel;
  }
  Errors::ExitWithError("Invalid PNG Colour type", -23);
  return -1;
}

constexpr uint8_t IO::PNG::ReconA(size_t ScanLine, size_t LineByteOffset, size_t Stride,
                              int BytesPerPixel, const uint8_t* Output) {
  return (LineByteOffset >= BytesPerPixel) ? Output[ScanLine * Stride + LineByteOffset - BytesPerPixel] : 0;
}

constexpr uint8_t IO::PNG::ReconB(size_t ScanLine, size_t LineByteOffset, size_t Stride,
                              const uint8_t* Output) {
  return (ScanLine > 0) ? Output[(ScanLine - 1) * Stride + LineByteOffset] : 0;
}

constexpr uint8_t IO::PNG::ReconC(size_t ScanLine, size_t LineByteOffset, size_t Stride,
                              int BytesPerPixel, const uint8_t* Output) {
  return (ScanLine > 0 && LineByteOffset >= BytesPerPixel) ? Output[(ScanLine - 1) * Stride + LineByteOffset - BytesPerPixel] : 0;
}

constexpr uint8_t IO::PNG::PaethPredictor(uint8_t A, uint8_t B, uint8_t C) {
  auto p = A + B - C;
  auto pa = abs(p - A);
  auto pb = abs(p - B);
  auto pc = abs(p - C);
  if (pa <= pb && pa <= pc)
    return A;
  else if (pb <= pc)
    return B;
  else
    return C;
}

void IO::ParseICOData(const std::vector<uint8_t>& Data, std::vector<std::shared_ptr<Image>>& OutImages) {
  uint16_t imageCount = u16LE(Data[4], Data[5]);

  OutImages.clear();
  OutImages.reserve(imageCount);

  for (uint16_t i = 0; i < imageCount; i++) {
    size_t entryOffset = ICO_DIR_OFFSET + (sizeof(ICONDIRENTRY) * i);
    ICONDIRENTRY entry{};

    entry.width = Data[entryOffset];
    entry.height = Data[entryOffset + 1];
    entry.paletteColours = Data[entryOffset + 2];
    if (Data[entryOffset + 3] != 0) Errors::ExitWithError("ICO reserved value is invalid", -9);
    entry.reserved = 0;
    entry.colourPlanes = u16LE(Data[entryOffset + 4], Data[entryOffset + 5]);
    entry.bitsPerPixel = u16LE(Data[entryOffset + 6], Data[entryOffset + 7]);
    entry.imageSize = u32LE(Data[entryOffset + 8], Data[entryOffset + 9], Data[entryOffset + 10], Data[entryOffset + 11]);
    entry.dataOffset = u32LE(Data[entryOffset + 12], Data[entryOffset + 13], Data[entryOffset + 14], Data[entryOffset + 15]);

    if (Data[entry.dataOffset] != 0x28) {
      if (CompareByteValues(std::vector<uint8_t>(Data.begin() + entry.dataOffset, Data.end()),
                            PNG_MAGIC_NUMBERS, PNG_MAGIC_NUMBER_BYTE_AMOUNT)) {
        // Ignore PNG's until i can figure out how to decode them
        continue;
        //OutImages.push_back(ParsePNGData(Data, entry.dataOffset));
      } else Errors::ExitWithError("Not a valid image format", -10);
    } else {
      BITMAPINFOHEADER bitmapInfoHeader{
        .headerSize = u32LE(Data[entry.dataOffset], Data[entry.dataOffset + 1],
                            Data[entry.dataOffset + 2], Data[entry.dataOffset + 3]),
        .width = s32LE(Data[entry.dataOffset + 4], Data[entry.dataOffset + 5],
                       Data[entry.dataOffset + 6], Data[entry.dataOffset + 7]),
        .height = abs(s32LE(Data[entry.dataOffset + 8], Data[entry.dataOffset + 9],
                      Data[entry.dataOffset + 10], Data[entry.dataOffset + 11]) / 2),
        .colourPlanes = u16LE(Data[entry.dataOffset + 12], Data[entry.dataOffset + 13]),
        .bitsPerPixel = u16LE(Data[entry.dataOffset + 14], Data[entry.dataOffset + 15]),
        .compressionMethod = u32LE(Data[entry.dataOffset + 16], Data[entry.dataOffset + 17],
                                   Data[entry.dataOffset + 18], Data[entry.dataOffset + 19]),
        .imageSize = u32LE(Data[entry.dataOffset + 20], Data[entry.dataOffset + 21],
                           Data[entry.dataOffset + 22], Data[entry.dataOffset + 23]),
        .hResolution = s32LE(Data[entry.dataOffset + 24], Data[entry.dataOffset + 25],
                             Data[entry.dataOffset + 26], Data[entry.dataOffset + 27]),
        .vResolution = s32LE(Data[entry.dataOffset + 28], Data[entry.dataOffset + 29],
                             Data[entry.dataOffset + 30], Data[entry.dataOffset + 31]),
        .colours = u32LE(Data[entry.dataOffset + 32], Data[entry.dataOffset + 33],
                         Data[entry.dataOffset + 34], Data[entry.dataOffset + 35]),
        .importantColours = u32LE(Data[entry.dataOffset + 36], Data[entry.dataOffset + 37],
                                  Data[entry.dataOffset + 38], Data[entry.dataOffset + 39])
      };

      OutImages.push_back(ParseBMPData(Data, bitmapInfoHeader, entry.dataOffset + bitmapInfoHeader.headerSize));
    }
  }
}

std::shared_ptr<IO::Image> IO::ParseBMPData(const std::vector<uint8_t>& Data, BITMAPINFOHEADER BitMapInfo, uint32_t Offset) {
  uint32_t rowSize = ceil((BitMapInfo.bitsPerPixel * BitMapInfo.width) / 32.f) * 4;
  uint32_t pixelArraySize = rowSize * BitMapInfo.height;
  std::shared_ptr<Image> returnPtr = std::make_shared<Image>();
  returnPtr->width = BitMapInfo.width;
  returnPtr->height = BitMapInfo.height;

  uint8_t* pixelArray = new uint8_t[BitMapInfo.width * BitMapInfo.height * sizeof(Pixel)];

  returnPtr->pixels = pixelArray;

  if (BitMapInfo.height > 0) {
    // Add 1 to row condition to stop underflowing unsigned value (by using >=)
    // while still accounting for last pixel row
    for (uint32_t row = (uint32_t)(BitMapInfo.height - 1); row + 1 > 0; row--) {
      uint32_t rowOffset = Offset + (rowSize * row);
      for (uint32_t pixel = 0; pixel < rowSize; pixel += (BitMapInfo.bitsPerPixel / 8)) {
        uint32_t tempOffset = pixel + rowOffset;
        if (pixel >= (BitMapInfo.bitsPerPixel / 8) * BitMapInfo.width) break;
        pixelArray[0] = Data[tempOffset + 2];
        pixelArray[1] = Data[tempOffset + 1];
        pixelArray[2] = Data[tempOffset];

        pixelArray[3] = (BitMapInfo.bitsPerPixel == 32) ? Data[tempOffset + 3] : 0xFF;

        pixelArray += 4;
      }
    }
  } else {
    for (uint32_t i = 0; i < pixelArraySize; i += (BitMapInfo.bitsPerPixel / 8)) {
      uint32_t tempOffset = Offset + i;
      pixelArray[0] = Data[tempOffset + 2];
      pixelArray[1] = Data[tempOffset + 1];
      pixelArray[2] = Data[tempOffset];

      pixelArray[3] = (BitMapInfo.bitsPerPixel == 32) ? Data[tempOffset + 3] : 0xFF;

      pixelArray += 4;
    }
  }

  return returnPtr;
}

int16_t IO::s16(int8_t byte0, int8_t byte1) {
  int16_t ret;
  ret = byte0;
  ret = (ret << 8) + byte1;
  return ret;
}

int32_t IO::s32(int8_t byte0, int8_t byte1, int8_t byte2, int8_t byte3) {
  int32_t ret;
  ret = byte0;
  ret = (ret << 8) + byte1;
  ret = (ret << 8) + byte2;
  ret = (ret << 8) + byte3;
  return ret;
}

int16_t IO::s16LE(int8_t byte1, int8_t byte0) {
  return u16(byte0, byte1);
}

int32_t IO::s32LE(int8_t byte3, int8_t byte2, int8_t byte1, int8_t byte0) {
  return u32(byte0, byte1, byte2, byte3);
}

uint16_t IO::u16(uint8_t byte0, uint8_t byte1) {
  uint16_t ret;
  ret = byte0;
  ret = (ret << 8) + byte1;
  return ret;
}

uint32_t IO::u32(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3) {
  uint32_t ret;
  ret = byte0;
  ret = (ret << 8) + byte1;
  ret = (ret << 8) + byte2;
  ret = (ret << 8) + byte3;
  return ret;
}

uint16_t IO::u16LE(uint8_t byte1, uint8_t byte0) {
  return u16(byte0, byte1);
}

uint32_t IO::u32LE(uint8_t byte3, uint8_t byte2, uint8_t byte1, uint8_t byte0) {
  return u32(byte0, byte1, byte2, byte3);
}

uint32_t IO::RGBA(uint8_t redByte, uint8_t greenByte, uint8_t blueByte, uint8_t alphaByte) {
  return u32(redByte, greenByte, blueByte, alphaByte);
}

} // namespace CustomIDE

