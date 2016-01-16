#pragma once

VOID FileCopyAsyncTest(LPCTSTR fileIn, LPCTSTR fileOut);
VOID CopyFolderAsyncTest(LPCTSTR folderIn, LPCTSTR folderOut);
VOID FileDumpAsyncTest(LPCTSTR fileIn);
VOID WriteAsyncTest(LPVOID buffer, DWORD length, LARGE_INTEGER offset);