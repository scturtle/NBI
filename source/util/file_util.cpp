#include "util/file_util.hpp"
#include "data/byte_buffer.hpp"
#include "util/error.hpp"

#include <memory>

extern "C" {
#include <switch/services/fs.h>
#include <switch/types.h>
}

static std::string FindCNMTFile(FsFileSystem &fileSystem, std::string path) {
  if (path.length() >= FS_MAX_PATH)
    THROW_FORMAT("Directory path is too long!");

  FsDir dir;
  Result rc = 0;
  rc = fsFsOpenDirectory(&fileSystem, path.c_str(), FsDirOpenMode_ReadFiles | FsDirOpenMode_ReadDirs, &dir);
  ASSERT_OK(rc, ("Failed to open directory " + path).c_str());

  s64 entryCount = 0;
  rc = fsDirGetEntryCount(&dir, &entryCount);
  ASSERT_OK(rc, "Failed to get entry count");

  auto dirEntries = std::make_unique<FsDirectoryEntry[]>(entryCount);

  s64 readEntries;
  rc = fsDirRead(&dir, &readEntries, entryCount, dirEntries.get());
  ASSERT_OK(rc, "Failed to read directory");
  assert(readEntries == entryCount);

  for (unsigned int i = 0; i < entryCount; i++) {
    FsDirectoryEntry dirEntry = dirEntries[i];
    std::string dirEntryName = dirEntry.name;

    if (dirEntry.type == FsDirEntryType_Dir) {
      auto found = FindCNMTFile(fileSystem, path + dirEntryName + "/");
      if (found != "")
        return found;
    } else if (dirEntry.type == FsDirEntryType_File) {
      auto foundExtension = dirEntryName.substr(dirEntryName.find(".") + 1);
      if (foundExtension == "cnmt")
        return path + dirEntryName;
    }
  }
  return "";
}

namespace tin::util {
nx::ncm::ContentMeta GetContentMetaFromNCA(std::string ncaPath) {

  Result rc = 0;
  if (ncaPath.length() >= FS_MAX_PATH)
    THROW_FORMAT("Directory path is too long!");

  // libnx expects a FS_MAX_PATH-sized buffer
  ncaPath.reserve(FS_MAX_PATH);

  FsFileSystem fileSystem;
  rc = fsOpenFileSystemWithId(&fileSystem, /*titleId=*/0, FsFileSystemType_ContentMeta, ncaPath.c_str(),
                              FsContentAttributes_All);

  ASSERT_OK(rc, ("Failed to open file system with id: " + ncaPath).c_str());

  // Find and read the cnmt file
  std::string cnmtFilePath = FindCNMTFile(fileSystem, "/");

  if (cnmtFilePath.length() >= FS_MAX_PATH)
    THROW_FORMAT("Directory path is too long!");

  // libnx expects a FS_MAX_PATH-sized buffer
  cnmtFilePath.reserve(FS_MAX_PATH);

  FsFile file;
  rc = fsFsOpenFile(&fileSystem, cnmtFilePath.c_str(), FsOpenMode_Read, &file);
  ASSERT_OK(rc, ("Failed to open file " + cnmtFilePath).c_str());

  s64 cnmtSize;
  rc = fsFileGetSize(&file, &cnmtSize);
  ASSERT_OK(rc, "Failed to get file size");

  auto cnmtBuf = std::make_unique<u8[]>(cnmtSize);

  u64 sizeRead;
  rc = fsFileRead(&file, 0, cnmtBuf.get(), cnmtSize, FsReadOption_None, &sizeRead);
  ASSERT_OK(rc, "Failed to read file");
  assert(sizeRead == (u64)cnmtSize);

  return nx::ncm::ContentMeta(cnmtBuf.get(), cnmtSize);
}
} // namespace tin::util
