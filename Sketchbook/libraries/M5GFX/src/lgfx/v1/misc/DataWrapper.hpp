/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [BSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
/----------------------------------------------------------------------------*/
#pragma once

#include <stdint.h>
#include <string.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------
  class LGFXBase;

  struct DataWrapper
  {
    constexpr DataWrapper(void) = default;
    virtual ~DataWrapper(void) = default;

    bool need_transaction = false;

    uint8_t read8(void)
    {
      uint8_t result;
      read(&result, 1);
      return result;
    }

    uint16_t read16(void)
    {
      uint16_t result;
      read(reinterpret_cast<uint8_t*>(&result), 2);
      return result;
    }

    uint32_t read32(void) {
      uint32_t result;
      read(reinterpret_cast<uint8_t*>(&result), 4);
      return result;
    }

    __attribute__ ((always_inline)) inline uint16_t read16swap(void) { return __builtin_bswap16(read16()); }
    __attribute__ ((always_inline)) inline uint32_t read32swap(void) { return __builtin_bswap32(read32()); }

    virtual bool open(__attribute__((unused)) const char* path) { return true; };
    virtual int read(uint8_t *buf, uint32_t len) = 0;
    virtual void skip(int32_t offset) = 0;
    virtual bool seek(uint32_t offset) = 0;
    virtual void close(void) = 0;
    virtual int32_t tell(void) = 0;

    __attribute__ ((always_inline)) inline void preRead(void) { if (fp_pre_read) fp_pre_read(parent); }
    __attribute__ ((always_inline)) inline void postRead(void) { if (fp_post_read) fp_post_read(parent); }
    __attribute__ ((always_inline)) inline bool hasParent(void) const { return parent; }
    LGFXBase* parent = nullptr;
    void (*fp_pre_read )(LGFXBase*) = nullptr;
    void (*fp_post_read)(LGFXBase*) = nullptr;
  };

//----------------------------------------------------------------------------

  struct PointerWrapper : public DataWrapper
  {
    void set(const uint8_t* src, uint32_t length = ~0) { _ptr = src; _length = length; _index = 0; }
    int read(uint8_t *buf, uint32_t len) override {
      if (len > _length - _index) { len = _length - _index; }
      memcpy(buf, &_ptr[_index], len);
      _index += len;
      return len;
    }
    void skip(int32_t offset) override { _index += offset; }
    bool seek(uint32_t offset) override { _index = offset; return true; }
    void close(void) override { }
    int32_t tell(void) override { return _index; }

  private:
    const uint8_t* _ptr = nullptr;
    uint32_t _index = 0;
    uint32_t _length = 0;
  };

//----------------------------------------------------------------------------

#if defined (SdFat_h)

  struct SdFatWrapper : public DataWrapper
  {
public:
    SdFatWrapper() : DataWrapper()
    {
      need_transaction = true;
      _fs = nullptr;
      _fp = nullptr;
    }

    SdBase<FsVolume>* _fs;
    FsFile *_fp;
    FsFile _file;

    SdFatWrapper(SdBase<FsVolume>& fs, FsFile* fp = nullptr) : DataWrapper(), _fs(&fs), _fp(fp) { need_transaction = true; }
    void setFS(SdBase<FsVolume>& fs) {
      _fs = &fs;
      need_transaction = true;
    }

    bool open(SdBase<FsVolume>& fs, const char* path)
    {
      setFS(fs);
      _file = fs.open(path, O_RDONLY);
      _fp = &_file;
      return _file;
    }
    bool open(const char* path) override
    {
      _file = _fs->open(path, O_RDONLY);
      _fp = &_file;
      return _file;
    }
    int read(uint8_t *buf, uint32_t len) override { return _fp->read(buf, len); }
    void skip(int32_t offset) override { _fp->seekCur(offset); }
    bool seek(uint32_t offset) override { return _fp->seekSet(offset); }
    void close(void) override { if (_fp) _fp->close(); }
    int32_t tell(void) override { return _fp->position(); }
  };

#endif

//----------------------------------------------------------------------------
 }
}


