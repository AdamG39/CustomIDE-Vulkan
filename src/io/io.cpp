#include "../helpers/errors/errors.hpp"
#include "io.hpp"
#include <fstream>
#include <cmath>

std::vector<char> ReadBinaryFile(const std::string& filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    ExitWithError("Failed to open file", -5);
  }

  size_t fileSize = (size_t) file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
}

bool CompareByteValues(const std::vector<char>& Obj1, const uint8_t* Obj2, size_t BytesToCompare) {
  if (Obj1.size() < BytesToCompare) ExitWithError("Not enough bytes to compare", -7);

  for (size_t i = 0; i < BytesToCompare; i++) {
    if (static_cast<uint8_t>(Obj1[i]) != Obj2[i]) return false;
  }

  return true;
}

// TODO: Change to return an array of shared pointers (pointer to pointers)
// and have return type be the amount of images returned
bool ReadImageFile(const std::string &filename, std::vector<std::shared_ptr<Image>>& OutImages) {
  std::vector<char> buffer = ReadBinaryFile(filename);

  if (buffer.size() == 0) ExitWithError("File buffer size is 0", -6);

  if (CompareByteValues(buffer, PNG_MAGIC_NUMBERS, PNG_MAGIC_NUMBER_BYTE_AMOUNT)) {
    //OutImages = ParsePNGData(buffer);
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

std::shared_ptr<Image> ParsePNGData(const std::vector<char>& Data, uint32_t Offset) {
  uint32_t offset = Offset + PNG_MAGIC_NUMBER_BYTE_AMOUNT;

  std::shared_ptr<Image> returnPtr = std::make_shared<Image>();

  PNGCHUNK chunk{};
  chunk.chunkSize = u32(Data[offset], Data[offset + 1], Data[offset + 2], Data[offset + 3]);
  chunk.chunkType = u32(Data[offset + 4], Data[offset + 5], Data[offset + 6], Data[offset + 7]);

  if (chunk.chunkType != PNG_IHDR_CHUNK_SIGNATURE) ExitWithError("No PNG image header found", -10);

  offset += sizeof(chunk.chunkSize) + sizeof(chunk.chunkType);

  PNGIMAGEHEADER imageHeader;
  imageHeader.width = u32(Data[offset], Data[offset + 1], Data[offset + 2], Data[offset + 3]);
  imageHeader.height = u32(Data[offset + 4], Data[offset + 5], Data[offset + 6], Data[offset + 7]);
  imageHeader.bitDepth = Data[offset + 8];
  imageHeader.colourType = Data[offset + 9];
  imageHeader.compressionMethod = Data[offset + 10];
  imageHeader.filterMethod = Data[offset + 11];
  imageHeader.interlaceMethod = Data[offset + 12];

  returnPtr->width = imageHeader.width;
  returnPtr->height = imageHeader.height;
  returnPtr->pixels = (uint8_t*)calloc((32 / imageHeader.bitDepth) * imageHeader.width * imageHeader.height, sizeof(uint8_t));

  offset += sizeof(PNGIMAGEHEADER) - sizeof(uint32_t) + 1;
  chunk = {};

  do {
    offset += sizeof(uint32_t); // Add an extra 4 bytes to get past the CRC
    offset += chunk.chunkSize;
    chunk = {};
    chunk.chunkSize = u32(Data[offset], Data[offset + 1], Data[offset + 2], Data[offset + 3]);
    chunk.chunkType = u32(Data[offset + 4], Data[offset + 5], Data[offset + 6], Data[offset + 7]);
    offset += sizeof(uint32_t) * 2;
  } while (chunk.chunkType != PNG_IDAT_CHUNK_SIGNATURE);

  // IDAT chunk found
  // Need to decode the data into valid pixel data
  InflateDecoder(std::vector(Data.begin() + offset, Data.begin() + offset + chunk.chunkSize), returnPtr->pixels);

  return returnPtr;
}

void InflateDecoder(const std::vector<char>& Data, uint8_t* Output) {
  ExitWithError("Unimpemented function", -99);
}

void ParseICOData(const std::vector<char>& Data, std::vector<std::shared_ptr<Image>>& OutImages) {
  uint16_t imageCount = u16LE(Data[4], Data[5]);

  OutImages.clear();
  OutImages.reserve(imageCount);

  for (uint16_t i = 0; i < imageCount; i++) {
    size_t entryOffset = ICO_DIR_OFFSET + (sizeof(ICONDIRENTRY) * i);
    ICONDIRENTRY entry{};

    entry.width = Data[entryOffset];
    entry.height = Data[entryOffset + 1];
    entry.paletteColours = Data[entryOffset + 2];
    if (Data[entryOffset + 3] != 0) ExitWithError("ICO reserved value is invalid", -9);
    entry.reserved = 0;
    entry.colourPlanes = u16LE(Data[entryOffset + 4], Data[entryOffset + 5]);
    entry.bitsPerPixel = u16LE(Data[entryOffset + 6], Data[entryOffset + 7]);
    entry.imageSize = u32LE(Data[entryOffset + 8], Data[entryOffset + 9], Data[entryOffset + 10], Data[entryOffset + 11]);
    entry.dataOffset = u32LE(Data[entryOffset + 12], Data[entryOffset + 13], Data[entryOffset + 14], Data[entryOffset + 15]);

    if (Data[entry.dataOffset] != 0x28) {
      if (CompareByteValues(std::vector<char>(Data.begin() + entry.dataOffset, Data.end()),
                            PNG_MAGIC_NUMBERS, PNG_MAGIC_NUMBER_BYTE_AMOUNT)) {
        // Ignore PNG's until i can figure out how to decode them
        continue;
        //OutImages.push_back(ParsePNGData(Data, entry.dataOffset));
      } else ExitWithError("Not a valid image format", -10);
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

std::shared_ptr<Image> ParseBMPData(const std::vector<char>& Data, BITMAPINFOHEADER BitMapInfo, uint32_t Offset) {
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

int16_t s16(int8_t byte0, int8_t byte1) {
  int16_t ret;
  ret = byte0;
  ret = (ret << 8) + byte1;
  return ret;
}

int32_t s32(int8_t byte0, int8_t byte1, int8_t byte2, int8_t byte3) {
  int32_t ret;
  ret = byte0;
  ret = (ret << 8) + byte1;
  ret = (ret << 8) + byte2;
  ret = (ret << 8) + byte3;
  return ret;
}

int16_t s16LE(int8_t byte1, int8_t byte0) {
  return u16(byte0, byte1);
}

int32_t s32LE(int8_t byte3, int8_t byte2, int8_t byte1, int8_t byte0) {
  return u32(byte0, byte1, byte2, byte3);
}

uint16_t u16(uint8_t byte0, uint8_t byte1) {
  uint16_t ret;
  ret = byte0;
  ret = (ret << 8) + byte1;
  return ret;
}

uint32_t u32(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3) {
  uint32_t ret;
  ret = byte0;
  ret = (ret << 8) + byte1;
  ret = (ret << 8) + byte2;
  ret = (ret << 8) + byte3;
  return ret;
}

uint16_t u16LE(uint8_t byte1, uint8_t byte0) {
  return u16(byte0, byte1);
}

uint32_t u32LE(uint8_t byte3, uint8_t byte2, uint8_t byte1, uint8_t byte0) {
  return u32(byte0, byte1, byte2, byte3);
}

uint32_t RGBA(uint8_t redByte, uint8_t greenByte, uint8_t blueByte, uint8_t alphaByte) {
  return u32(redByte, greenByte, blueByte, alphaByte);
}

