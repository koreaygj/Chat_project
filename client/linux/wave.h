#pragma once
#pragma warning(disable : 4996)
#include <iostream>
#include <fstream>
#include <WinSock2.h>
using namespace std;
// wav에 접근하기 위해서는 필요합니다.
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ws2_32")
// Wave 객체
class Wave
{
private:
  // 음성 포멧 지정
  WAVEFORMATEX m_format;
  // 음성 입력 데이터 구조체
  WAVEHDR m_waveInHdr;
  // 음성을 입력 장치 구조체
  HWAVEIN m_hWaveIn;
  // 음성을 출력 데이터 구조체
  WAVEHDR m_waveOutHdr;
  // 음성을 출력 장치 구조체
  HWAVEOUT m_hWaveOut;
  // 소켓
  SOCKET m_sock;
  // 초기화
  void Init();
  // 소켓 Receive 대기 함수
  void Receive();

public:
  // 생성자
  Wave();
  // 서버 Listen 함수
  void Listen(int);
  // 클라이언트 Connect함수
  void Connect(const char *, int);
};