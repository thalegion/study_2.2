#include <windows.h> 
#include <windowsx.h>
#include <tchar.h> 
#include <string.h>
#include "resource.h"

#define FILENAME __TEXT(" FILEREV.DAT")

// ����� __TEXT() ��������� ������ � ANSI ������� ���
// ����������� � ������ UNICODE � ����������� �� ������� 
// ����������: ��������� ��� ��� ������ _UNICODE  

int WINAPI WinMain (HINSTANCE hinstExe,	HINSTANCE hinstPrev, 
					          LPSTR lpszCmdLine, 	int nCmdShow) 
{
	// ���������� ��������� ����������:

	HANDLE hFile;        // ��� ����������� ������� "����"
  HANDLE hFileMap;     // ��� ����������� ������� '������������ ����'
	LPVOID lpvFile;      // ��� ������ ������� � �������� ������������
                       // ���� ����� ������������ ����

	LPSTR  lpchANSI;     // ��������� �� ANSI ������
  LPWSTR lpchUNI;      // ��������� �� ������ � UNICODE 
	DWORD  dwFileSize;   // ��� �������� ������� ����� 
	LPTSTR lpszCmdLineT; // ��������� �� ��������� ������ 
                       // � ANSI ��� UNICODE
  BOOL fIsTextUnicode = FALSE; // ���� ������������� ������ � UNICODE

	
	STARTUPINFO si={0};    // ��������� ��� �������
	PROCESS_INFORMATION pi;// CreateProcess 

	
	// �������� �� ��������� ������ ��� �������������� �����.
  
  // lpszCmdLineT =lpszCmdLine; // ������ ��� Windows 95/98

  // ���� ����� ������������ ���� �� ��������� NT, �� ������ 
  // ������������ �������� lpszCmdLine ������� WinMain, ��� 
  // ��� ��� ������ ��������� �� ANSI - ������. 
  // ������� GetCmdLine ��������� �������� ��������� ������ 
  // � ANSI ��� UNICODE � ����������� �� ����, ��� �� �����������
  // ��������� ( �������� ������ ##define _UNICODE ��� ��� )
  lpszCmdLineT=GetCommandLine();
  if(*lpszCmdLineT++ == __TEXT('"'))   
    lpszCmdLineT= _tcschr( lpszCmdLineT,  __TEXT('"'));
 
  lpszCmdLineT= _tcschr( lpszCmdLineT,  __TEXT(' '));
  
	// _tcschr() - ����������� �������, ���������� � ����������� ��
  // ������� ���������� �� strchr(), wcschr() ���  _mbschr().
  // ���� � ������ �������� ������ � ���������� ��������� �� ����.
  // � ������ ������ ���� ������ ������.

  if(lpszCmdLineT !=NULL)
    {
    //��������� ������ ��������
    while(*lpszCmdLineT==__TEXT(' '))
      lpszCmdLineT++;
    }
	
  if((lpszCmdLineT == NULL) || (*lpszCmdLineT ==0))
		{
    // ������� �� �������. ��������� ����� ���� ��� ��������� �� 
    // ����� ������ (������� ���� � �����). ������ ���������� �����
    // ����� ������������ ����� �� �������. �������� �� ������.
		MessageBox( NULL, 
                __TEXT("You must enter a filename on ")
					      __TEXT("the command line."), // ������ ���������
					      __TEXT("FileRev"), // ������ ��������� ����
                MB_OK );
		return(0);
		}

  // ����� �� ��������� ������� ����, �������� ��� � ����� ����,
  // ��� �������� ��������� ����� ������������ ���� ��������� FILENAME.
	
  if (!CopyFile(lpszCmdLineT, FILENAME, FALSE)) 
		{
		// ����������� �� �������
		MessageBox( NULL, __TEXT("New file could not be created."),
					      __TEXT("FileRev"), MB_OK);
		return (0);
		}

	// ��������� ���� ��� ������ � ������. ��� ����� ������� ������
  // ���� "����". � ����������� �� ��������� ���������� ������� 
  // CreateFile ���� ��������� ������������ ����, ���� ������� �����.
  // ������ ��� ������� ����� �������������� ��� �������� �����,
  // ������������� � ������. ������� ���������� ���������� ���������
  // ������� ����, ��� ��� ������ INVALID_HANDLE_VALUE.

	hFile = CreateFile(	
          FILENAME,  // ��������� �� ������ � ������� �����
          GENERIC_WRITE | GENERIC_READ, // ��������� ����� �������
					0,   //  ����� ����������,0 - ������������� ������ � �����.       
          NULL,// LPSECURITY_ATTRIBUTES=0 - ������ �� �����������.
          OPEN_EXISTING,//���� ���� �� ����������, �� ���������� ������.
          FILE_ATTRIBUTE_NORMAL,//�������� �������� ��� ���� 
          NULL //�� ������ ��� ������� ���� "����"
          );

	if (hFile == INVALID_HANDLE_VALUE) 
		{  // ������� ���� �� �������
		MessageBox(	NULL, __TEXT("File could not be opened."),
					      __TEXT("FileRev"), MB_OK);
		return(0);
		}

	// ������ ������ �����. ������ �������� NULL, ��� ��� ��������������,
  // ��� ���� ������ 4 ��.
	dwFileSize = GetFileSize(hFile, NULL);

	// ������� ������ "������������ ����". �� - �� 1 ���� ������, ���
	// ������ �����, ����� ����� ���� �������� � ����� ����� ������� 
  // ������  � �������� � ������ ��� � ����-��������������� �������.
  // ��������� ���� ��� ���������� �������� ���� ANSI - ��� Unicode
  // - �������, ����������� ���� �� �������� ������� ������� WCHAR
	
	hFileMap = CreateFileMapping(	
                    hFile,  // ���������� ������������ ������� "����" 
                    NULL,   // 
                    PAGE_READWRITE, // �������� ������ ������� 
									  0,  // LPSECURITY_ATTRIBUTES
                    dwFileSize+sizeof(WCHAR),   //������� 32 �������
                    NULL  // � ������� 32 ������� ������� �����.
                    );
	if (hFileMap == NULL) 
		{	// ������� ������ "������������ ����" �� �������
		MessageBox(	NULL, __TEXT("File map could not be opened."),
					      __TEXT("FileRev"),    MB_OK);
	  CloseHandle(hFile);// ����� ������� ��������� �������� �������
		return (0);
		}

	// ��������� ����������� ����� �� ����������� �������� ������������ �
  // �������� �����, ������� � �������� ������������� ����� ����� � ������.
  
  lpvFile = MapViewOfFile(
                hFileMap, // ���������� ������� "������������ ����" 
                FILE_MAP_WRITE, // ����� �������
                0, // ������� 32 ������� �������� �� ������ �����, 
                0, // ������� 32 ������� �������� �� ������ �����
                0  // � ���������� ������������ ����. 0 - ���� ����.
                );

	if (lpvFile == NULL)
		{// ������������� ������� ������������� ����� �� �������
		MessageBox(	NULL, __TEXT("Could   not map view of tile."),
					      __TEXT("FileRev"), MB_OK);
		CloseHandle (hFileMap);// ����� ������� ��������� �������� �������
		CloseHandle(hFile);
		return(0);
		}

  // ���������� �������� �� ���� ����� � UNICODE- ��� ANSI-���������.

  fIsTextUnicode = IsTextUnicode(lpvFile,dwFileSize,NULL);

  if( fIsTextUnicode )
    { // � ���� ����� ����� ������������ UNICODE - �������
    
		// ���������� � ����� ����� ������� ������
		lpchUNI = (LPWSTR) lpvFile;
		lpchUNI[dwFileSize] = 0;

		// "��������������" ���������� ����� ��������
		_wcsrev(lpchUNI);

		// ����������� ��� ���������� ����������� �������� "\n\r" ������� � "\r\n",
		// ����� ��������� ���������� ������������������ ����� "������� �������"
    // "������� ������", ����������� ������ � ��������� �����

		lpchUNI = (LPWSTR) wcschr(lpchUNI,(int)'\n'); // ������� ����� ������� '\n'
		while (lpchUNI != NULL) //���� �� �������� 0 � ����� ����� 
			{
			*lpchUNI++ =(int)'\r';//�������� '\n' �� '\r' � ����������� ��������� 
			*lpchUNI++ =(int)'\n';//�������� '\r' �� '\n' � ����������� ���������
			 lpchUNI = (LPWSTR) wcschr(lpchUNI,(int)'\n');// ������� ����� ���������� '\n'
			}
    }
	else
    { // � ������ ����� ���������� ANSI-������� ��� ������ � �������

		// ���������� � ����� ����� ������� ������
		lpchANSI = (LPSTR) lpvFile;
		lpchANSI[dwFileSize] = 0;

		// "��������������" ���������� ����� ��������
		_strrev(lpchANSI);

		// ����������� ��� ���������� ����������� �������� "\n\r" ������� � "\r\n",
		// ����� ��������� ���������� ������������������ ����� "������� �������"
    // "������� ������", ����������� ������ � ��������� �����
		
    lpchANSI = strchr(lpchANSI,(int) '\n');   // ������� ����� ������� '\n'
		while (lpchANSI != NULL) //���� �� ������� ��� ������� '\n'
			{
			  *lpchANSI  =(int)'\r'; // �������� '\n' �� '\r'
         lpchANSI++;           // ����������� ���������
			  *lpchANSI++ =(int)'\n';// �������� '\r' �� '\n' � ����������� ���������
			   lpchANSI  = strchr(lpchANSI,(int)'\n');  // ���� ������
			}
    }

	// ������� ��� ����� �����������
  // ��������� ������������� ����� � ���� ��������� ������������
	UnmapViewOfFile(lpvFile);
  
  // ��������� ������� ������ �� ������ ���� "������������ ����"
	CloseHandle(hFileMap);
	
	// ������� ����������� ����� �������� ������� ����.��� �����
	// ���������� ��������� ����� � ����� �� ������� ����,
	// � ����� ���� ������� ���������� � ���� ����� ����� �����
	
  SetFilePointer(hFile, dwFileSize, NULL, FILE_BEGIN);
  SetEndOfFile(hFile);
	// SetEndOfFlle ����� �������� ����� �������� ����������� �������
  // ���� "������������ ����"
	
	CloseHandle(hFile);// ��������� ������� ������ �� ������ ���� "����"

	// ��������� NOTEPAD � ��������� � ���� ��������� ����,
  // ����� ������� ����� ����� ������
	si.cb = sizeof (si);// ��������� ���� ������� ��������� si
	si.wShowWindow = SW_SHOW;// ������ ����� ������ ���� NOTEPAD
	si.dwFlags = STARTF_USESHOWWINDOW;// ������������� ���� - ���������
                                    // �������� ���� wShowWindow
	if( CreateProcess(	NULL,  __TEXT("NOTEPAD.EXE") FILENAME,
						          NULL, NULL, FALSE, 0, 
						          NULL, NULL, &si, &pi) )
    {
    // ���� ������� ������, ����������� 
    // ����������� ������ � ��������	
    CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		}
	return( 0 );
}


