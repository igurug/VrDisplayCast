#include <Ws2tcpip.h>
#include <WinSock2.h>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/opencv.hpp>

#pragma once
using namespace std;
using namespace cv;

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "Ws2_32.lib")

#ifndef VR_RES_ORIGINAL
#define VR_RES_ORIGINAL 333
#define VR_RES_1280		444
#define VR_RES_1024		555
#define VR_RES_800		666
#endif

bool run;
int resolution = VR_RES_1024;
thread* server;
char ipaddr[100];

void setResolution(int resNew)
{
	resolution = resNew;
}

Mat hwnd2mat()
{
	HWND hwnd = GetDesktopWindow();
	//namedWindow("output", WINDOW_NORMAL);
	RECT windowsize;
	GetWindowRect(hwnd, &windowsize);

	HDC hwindowDC;



	int height, width, srcheight, srcwidth;
	//HBITMAP hbwindow;
	Mat src;
	BITMAPINFOHEADER  bi;

	//hwindowDC = GetDC(hwnd);
	//hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetProcessDPIAware();
	float dpiX, dpiY;
	
	HDC hdc = GetWindowDC(hwnd);
	
	dpiX = static_cast<FLOAT>(GetDeviceCaps(hdc, LOGPIXELSX));
	dpiY = static_cast<FLOAT>(GetDeviceCaps(hdc, LOGPIXELSY));

	HDC hdcMem = CreateCompatibleDC(hdc);
//	int cx = dpiX / 96.0f*(float)GetDeviceCaps(hdc, HORZRES);
//	int cy = dpiY / 96.0f*(float)GetDeviceCaps(hdc, VERTRES);
	int cx = windowsize.right;
	int cy = windowsize.bottom;
	WCHAR bufc[1000];
	wsprintf(bufc, L"%i, %i\n", (int)dpiX, (int)dpiY);
	//OutputDebugString(bufc);
	HBITMAP hbitmap(NULL);
	hbitmap = CreateCompatibleBitmap(hdc, cx, cy);
	SelectObject(hdcMem, hbitmap);
	BitBlt(hdcMem, 0, 0, cx, cy, hdc, 0, 0, SRCCOPY);

	CURSORINFO cursor0 = { sizeof(cursor0) };
	GetCursorInfo(&cursor0);
	const int x = cursor0.ptScreenPos.x;
	const int y = cursor0.ptScreenPos.y;

	DrawIcon(hdcMem, x, y, cursor0.hCursor);


	

	src.create(cy, cx, CV_8UC4);


	
	bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
	bi.biWidth = cx;
	bi.biHeight = -cy;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	GetDIBits(hdcMem, hbitmap, 0, cy, src.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

	switch(resolution)
	{
	case VR_RES_1280:
		resize(src, src, Size(1280, 1024), 0, 0, 1);
		break;

	case VR_RES_1024:
		resize(src, src, Size(1024, 768), 0, 0, 1);
		break;

	case VR_RES_800:
		resize(src, src, Size(800, 600), 0, 0, 1);
		break;

	}
	//resize(src, src, Size(1280, 1024), 1, 1, 1);
	//rotate(src, src, ROTATE_90_CLOCKWISE);


	// avoid memory leak

	DeleteObject(hbitmap);

	ReleaseDC(0, hdc);
	ReleaseDC(0, hdcMem);
	DeleteDC(hdcMem);


	return src;
}

std::wstring get_utf16(const std::string &str, int codepage)
{
	if (str.empty()) return std::wstring();
	int sz = MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), 0, 0);
	std::wstring res(sz, 0);
	MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), &res[0], sz);
	return res;
}

void hexchar(unsigned char c, unsigned char &hex1, unsigned char &hex2)
{
	hex1 = c / 16;
	hex2 = c % 16;
	hex1 += hex1 <= 9 ? '0' : 'a' - 10;
	hex2 += hex2 <= 9 ? '0' : 'a' - 10;
}

typedef unsigned char uchar;
static const std::string b = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";//=
static std::string base64_encode(const std::string &in) {
	std::string out;

	int val = 0, valb = -6;
	for (uchar c : in) {
		val = (val << 8) + c;
		valb += 8;
		while (valb >= 0) {
			out.push_back(b[(val >> valb) & 0x3F]);
			valb -= 6;
		}
	}
	if (valb > -6) out.push_back(b[((val << 8) >> (valb + 8)) & 0x3F]);
	while (out.size() % 4) out.push_back('=');
	return out;
}


SOCKET init_socket(string address, int port)
{
	int sock = 0, valread;
	struct sockaddr_in serv_addr;

	WSADATA wsa;
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		OutputDebugString(L"Socket error...\n");
		MessageBox(0, L"Error", L"Failed to initialize socket", MSGF_MESSAGEBOX);
		//printf("Failed. Error Code : %d", WSAGetLastError());
		return 0;
	}

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		OutputDebugString(L"Socket error 1...\n");
		MessageBox(0, L"Cannot open socket", L"Run mobile applicaion first", MSGF_MESSAGEBOX);
		return 0;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	// Convert IPv4 and IPv6 addresses from text to binary form 

	if (inet_pton(AF_INET, address.c_str(), &serv_addr.sin_addr) <= 0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return 0;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return 0;
	}
	return sock;
}

int send_socket(SOCKET sock, string dat)
{
	string sdata = base64_encode(dat);

	char*  data = (char*)sdata.c_str();
	//	char*  data = (char*)dat.c_str();
	//	DWORD data_len = dat.size();
	DWORD data_len = strlen(data);

	//cout << data << endl;
	cout << data_len << endl;


	char slen[100];
	sprintf_s(slen, "%i\n", data_len);
	int slen_len = strlen(slen);




	send(sock, data, data_len, 0);
	printf("Data sent\n");
	//recv(sock, buffer, 1024, 0);
	const char* ending = "\n---end---\n";
	int result = send(sock, ending, strlen(ending), 0);

	return result;
}



void server_process()
{
	SOCKET sock;
	sock = init_socket(ipaddr, 8800);
	//sock = init_socket("192.168.43.208", 8800);
	OutputDebugString(L"Attempting to create socket...\n");
	//sock = init_socket("10.74.57.7", 8800);
	//sock = init_socket("192.168.1.103", 8800);
	if (sock <= 0)
	{
		OutputDebugString(L"Socket creation failed!\n");
		return;
	}
	

	int key = 0;
	vector<uchar> buf;
	vector<int> compression_params;
	compression_params.push_back(IMWRITE_JPEG_QUALITY);
	compression_params.push_back(30);
	Mat src;

	//getCursor();
	OutputDebugString(L"Socket ready...\n");
	char testbuf;
	//int error_code_size = sizeof(error_code);
	while (run)
	{
	//	OutputDebugString(L"step...\n");
		//hwndDesktop = GetDesktopWindow();
		src = hwnd2mat();

		// you can do some image processing here
//imshow("output", src);



		imencode(".jpg", src, buf, compression_params);
		src.release();

		string str(buf.begin(), buf.end());
		buf.clear();


		//thread t1(send_socket, sock, str);
		//t1.join();




		if (send_socket(sock, str) <= 0)
			break;

		
		

		//key = waitKey(1); // you can change wait time
	}
	OutputDebugString(L"Finishing screen casting...\n");

}


void start_server()
{
	run = true;
	
	server = new thread(server_process);
	//server->join();
	//server_process();
	OutputDebugString(L"Thread started...\n");
}
void stop_server()
{

	run = false;
}