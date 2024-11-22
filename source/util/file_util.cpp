/*
Copyright (c) 2017-2018 Adubbz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "util/file_util.hpp"

#include <memory>

#include "data/byte_buffer.hpp"
#include "nx/fs.hpp"
#include "util/title_util.hpp"

namespace {
std::string FindCNMTFile(nx::fs::IFileSystem &fileSystem, std::string path) {
  nx::fs::IDirectory dir = fileSystem.OpenDirectory(path, FsDirOpenMode_ReadFiles | FsDirOpenMode_ReadDirs);

  u64 entryCount = dir.GetEntryCount();
  auto dirEntries = std::make_unique<FsDirectoryEntry[]>(entryCount);
  dir.Read(0, dirEntries.get(), entryCount);

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
} // namespace

namespace tin::util {
// TODO: do this manually so we don't have to "install" the cnmt's
nx::ncm::ContentMeta GetContentMetaFromNCA(const std::string &ncaPath) {
  // Create the cnmt filesystem
  nx::fs::IFileSystem cnmtNCAFileSystem;
  cnmtNCAFileSystem.OpenFileSystemWithId(ncaPath, FsFileSystemType_ContentMeta, 0);

  // Find and read the cnmt file
  std::string cnmtFilePath = FindCNMTFile(cnmtNCAFileSystem, "/");
  nx::fs::IFile cnmtFile = cnmtNCAFileSystem.OpenFile(cnmtFilePath);
  u64 cnmtSize = cnmtFile.GetSize();

  tin::data::ByteBuffer cnmtBuf;
  cnmtBuf.Resize(cnmtSize);
  cnmtFile.Read(0x0, cnmtBuf.GetData(), cnmtSize);

  return nx::ncm::ContentMeta(cnmtBuf.GetData(), cnmtBuf.GetSize());
}
} // namespace tin::util
