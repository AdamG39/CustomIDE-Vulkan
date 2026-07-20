#ifndef CUSTOM_IO_H
#define CUSTOM_IO_H

#include <vector>
#include <memory>
#include <string>
#include "../helpers/errors/errors.hpp"

namespace CustomIDE::IO {

// Image file magic numbers
constexpr size_t PNG_MAGIC_NUMBER_BYTE_AMOUNT = 8;
constexpr size_t BMP_MAGIC_NUMBER_BYTE_AMOUNT = 2;
constexpr size_t ICO_MAGIC_NUMBER_BYTE_AMOUNT = 4;

constexpr uint8_t PNG_MAGIC_NUMBERS[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
constexpr uint8_t BMP_MAGIC_NUMBERS[] = { 0x42, 0x4D };
constexpr uint8_t ICO_MAGIC_NUMBERS[] = { 0x00, 0x00, 0x01, 0x00 };

// PNG Specific macros
constexpr size_t PNG_IHDR_CHUNK_SIGNATURE                    = 0x49484452;
constexpr size_t PNG_sRGB_CHUNK_SIGNATURE                    = 0x73524742;
constexpr size_t PNG_IDAT_CHUNK_SIGNATURE                    = 0x49444154;
constexpr size_t PNG_IEND_CHUNK_SIGNATURE                    = 0x49454E44;

// ICO Specific macros
constexpr size_t ICO_DIR_OFFSET = 6;
constexpr size_t ICO_COMPRESSION_MEDTHOD_BI_RGB              = 0;
constexpr size_t ICO_COMPRESSION_MEDTHOD_BI_RLE8             = 1;
constexpr size_t ICO_COMPRESSION_MEDTHOD_BI_RLE4             = 2;
constexpr size_t ICO_COMPRESSION_MEDTHOD_BI_BITFIELDS        = 3;
constexpr size_t ICO_COMPRESSION_MEDTHOD_BI_JPEG             = 4;
constexpr size_t ICO_COMPRESSION_MEDTHOD_BI_PNG              = 5;
constexpr size_t ICO_COMPRESSION_MEDTHOD_BI_ALPHABITFIELDS   = 6;
constexpr size_t ICO_COMPRESSION_MEDTHOD_BI_CMYK             = 11;
constexpr size_t ICO_COMPRESSION_MEDTHOD_BI_CMYKRLE8         = 12;
constexpr size_t ICO_COMPRESSION_MEDTHOD_BI_CMYKRLE4         = 13;

using Pixel = uint32_t;

int16_t s16(int8_t byte0, int8_t byte1);
int32_t s32(int8_t byte0, int8_t byte1, int8_t byte2, int8_t byte3);
int16_t s16LE(int8_t byte1, int8_t byte0);
int32_t s32LE(int8_t byte3, int8_t byte2, int8_t byte1, int8_t byte0);

uint16_t u16(uint8_t byte0, uint8_t byte1);
uint32_t u32(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3);
uint16_t u16LE(uint8_t byte1, uint8_t byte0);
uint32_t u32LE(uint8_t byte3, uint8_t byte2, uint8_t byte1, uint8_t byte0);

uint32_t RGBA(uint8_t redByte, uint8_t greenByte, uint8_t blueByte, uint8_t alphaByte);

struct ICONDIRENTRY {
  uint8_t width;
  uint8_t height;
  uint8_t paletteColours;
  uint8_t reserved;
  uint16_t colourPlanes;
  uint16_t bitsPerPixel;
  uint32_t imageSize;
  uint32_t dataOffset;
};

struct BITMAPFILEHEADER {
  uint16_t magicNumbers = 0x424D;
  uint32_t size;
  uint16_t reserved0;
  uint16_t reserved1;
  uint32_t offset;
};

struct BITMAPINFOHEADER {
  uint32_t headerSize;
  int32_t width;
  int32_t height;
  uint16_t colourPlanes;
  uint16_t bitsPerPixel;
  uint32_t compressionMethod;
  uint32_t imageSize;
  int32_t hResolution;
  int32_t vResolution;
  uint32_t colours;
  uint32_t importantColours;
};

struct PNGCHUNK {
  uint32_t chunkSize;
  uint32_t chunkType;
};

struct PNGIMAGEHEADER {
  uint32_t width;
  uint32_t height;
  uint8_t bitDepth;
  uint8_t colourType;
  uint8_t compressionMethod;
  uint8_t filterMethod;
  uint8_t interlaceMethod;
};

struct Image {
  uint32_t width;
  uint32_t height;
  uint8_t* pixels;

  ~Image() {
    free(pixels);
  }
};

std::vector<uint8_t> ReadBinaryFile(const std::string& filename);

std::string ReadTextFile(const std::string& filename);

void WriteTextFile(const std::string& filename, const std::string& content);

bool CompareByteValues(const std::vector<uint8_t>& Obj1, const uint8_t* Obj2, size_t BytesToCompare);

bool ReadImageFile(const std::string &filename, std::vector<std::shared_ptr<Image>>& OutImages);

std::shared_ptr<Image> ParsePNGData(const std::vector<uint8_t>& Data, uint32_t Offset);

int InflateDecoder(const std::vector<uint8_t>& Data, std::vector<uint8_t>& Output);

namespace PNG {
  enum class ColourType {
    Grayscale = 0,
    Truecolour = 2,
    Indexed = 3,
    GrayscaleAlpha = 4,
    TruecolourAlpha = 6
  };

  constexpr int GetImageBitsPerPixel(uint8_t BitsPerChannel, ColourType ColourMode);

  constexpr uint8_t ReconA(size_t ScanLine, size_t LineByteOffset, size_t Stride, int BytesPerPixel, const uint8_t* Output);
  constexpr uint8_t ReconB(size_t ScanLine, size_t LineByteOffset, size_t Stride, const uint8_t* Output);
  constexpr uint8_t ReconC(size_t ScanLine, size_t LineByteOffset, size_t Stride, int BytesPerPixel, const uint8_t* Output);

  constexpr uint8_t PaethPredictor(uint8_t A, uint8_t B, uint8_t C);
} // namespace PNG

void ParseICOData(const std::vector<uint8_t>& Data, std::vector<std::shared_ptr<Image>>& OutImages);

std::shared_ptr<Image> ParseBMPData(const std::vector<uint8_t>& Data, BITMAPINFOHEADER BitMapInfo, uint32_t Offset);

} // namespace CustomIDE::IO

#endif

