#include "Wave.h"
// Callback 함수, 음성 녹음 시간을 지정해 놓고 녹음하는 게 아니기 때문에 동적인 콜백 함수가 필요한다.
void CALLBACK waveInProc(HWAVEIN hWaveIn, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
  switch (uMsg)
  {
  // 데이터가 호출되면 메시지가 WIM_DATA로 호출된다.
  case WIM_DATA:
  {
    // 음성 데이터 구조체 취득
    WAVEHDR *WaveInHdr = (WAVEHDR *)dwParam1;
    // 소켓을 가져온다.
    SOCKET pClient = (SOCKET)WaveInHdr->dwUser;
    // 음성 데이터 전송
    send(pClient, WaveInHdr->lpData, WaveInHdr->dwBytesRecorded, 0);
    // 장치에 녹음 준비를 알리는 함수
    if (waveInPrepareHeader(hWaveIn, WaveInHdr, sizeof(WAVEHDR)))
    {
      // 에러 콘솔 출력
      cout << "waveInPrepareHeader error" << endl;
      // 장치 닫기
      waveInClose(hWaveIn);
      return;
    }
    // 장치에 메모리를 할당을 하는 함수
    if (waveInAddBuffer(hWaveIn, WaveInHdr, sizeof(WAVEHDR)))
    {
      // 에러 콘솔 출력
      cout << "waveInPrepareHeader error" << endl;
      // 장치 닫기
      waveInClose(hWaveIn);
      return;
    }
  }
  break;
  }
}
// 생성자
Wave::Wave()
{
  // 멤버 변수 초기화
  memset(&m_format, 0x00, sizeof(WAVEFORMATEX));
  memset(&m_waveInHdr, 0x00, sizeof(WAVEHDR));
  memset(&m_waveOutHdr, 0x00, sizeof(WAVEHDR));
  memset(&m_hWaveIn, 0x00, sizeof(HWAVEIN));
  memset(&m_hWaveOut, 0x00, sizeof(HWAVEOUT));
  // 포멧 설정
  m_format.wFormatTag = WAVE_FORMAT_PCM;
  // 채널 설정
  m_format.nChannels = 1;
  // 샘플링 데이터 횟수.
  // nSamplesPerSec에는 8.0 kHz, 11.025 kHz, 22.05 kHz, 44.1 kHz가 있습니다.
  m_format.nSamplesPerSec = 8000;
  // 1회 샘플링에 사용되어지는 데이터 비트수, 8 또는 16
  m_format.wBitsPerSample = 8;
  // 샘플링에 사용되는 바이트 단위의 메모리 크기
  m_format.nBlockAlign = m_format.nChannels * (m_format.wBitsPerSample / 8);
  // WAVE_FORMAT_PCM에서는 무시되는 값
  m_format.cbSize = 0;
  // WAVE_FORMAT_PCM이라면 무압축이기 때문에 nSamplesPerSec와 같을 것이다.
  m_format.nAvgBytesPerSec = m_format.nSamplesPerSec;
  // 라이브러리에서 실제 녹음된 사이즈를 구하는 함수(사용자가 사용하는 값이 아님)
  m_waveInHdr.dwBytesRecorded = 0;
  m_waveOutHdr.dwBytesRecorded = 0;
  // 라이브러리에서 callback 함수 사용시 사용되는 status flag
  m_waveInHdr.dwFlags = 0;
  // 반복 재생시 사용됨 (사용하지 않는다.)
  m_waveInHdr.dwLoops = 0;
  // 예약 재생시 사용됨 (사용하지 않는다.)
  m_waveInHdr.reserved = 0;
  // 라이브러리에서 callback 함수 사용시 사용되는 status flag
  m_waveOutHdr.dwFlags = 0;
  // 반복 재생시 사용됨 (사용하지 않는다.)
  m_waveOutHdr.dwLoops = 0;
  // 예약 재생시 사용됨 (사용하지 않는다.)
  m_waveOutHdr.reserved = 0;
  // 데이터 버퍼 설정(입력 장치)
  m_waveInHdr.dwBufferLength = m_format.nAvgBytesPerSec;
  m_waveInHdr.lpData = (char *)malloc(m_waveInHdr.dwBufferLength);
  // 데이터 버퍼 설정(출력 장치)
  m_waveOutHdr.dwBufferLength = m_format.nAvgBytesPerSec;
  m_waveOutHdr.lpData = (char *)malloc(m_waveInHdr.dwBufferLength);
}
// 초기화 함수
void Wave::Init()
{
  // m_waveInHdr.dwUser에 소켓 정보를 넣는다.
  m_waveInHdr.dwUser = (DWORD_PTR)m_sock;
  // 입력 장치 설정
  if (waveInOpen(&m_hWaveIn, waveInGetNumDevs() - 1 /*WAVE_MAPPER*/, &m_format, (DWORD_PTR)waveInProc, 0, CALLBACK_FUNCTION))
  {
    // 에러 콘솔 출력
    cout << "Failed to open waveform input device." << endl;
    throw -1;
  }
  // 장치에 녹음 준비를 알리는 함수
  if (waveInPrepareHeader(m_hWaveIn, &m_waveInHdr, sizeof(WAVEHDR)))
  {
    // 에러 콘솔 출력
    cout << "waveInPrepareHeader error" << endl;
    throw -1;
  }
  // 버퍼 설정
  if (waveInAddBuffer(m_hWaveIn, &m_waveInHdr, sizeof(WAVEHDR)))
  {
    // 에러 콘솔 출력
    cout << "waveInAddBuffer error" << endl;
    throw -1;
  }
  // 출력 장치 설정
  if (waveOutOpen(&m_hWaveOut, waveInGetNumDevs() - 1 /*WAVE_MAPPER*/, &m_format, 0, 0, WAVE_FORMAT_DIRECT))
  {
    // 에러 콘솔 출력
    cout << "Failed to open waveform output device." << endl;
    throw;
  }
}
// 소켓으로 데이터를 받으면 스피커로 출력하는 함수
void Wave::Receive()
{
  // 무한 루프
  while (true)
  {
    // 데이터 받기
    recv(m_sock, m_waveOutHdr.lpData, m_waveOutHdr.dwBufferLength, 0);
    // 출력 장치 재설정
    if (waveOutReset(m_hWaveOut))
    {
      // 에러 콘솔 출력
      cout << "waveOutReset error" << endl;
      // 장치 닫기
      throw -1;
    }
    // 장치에 출력 준비를 알리는 함수
    if (waveOutPrepareHeader(m_hWaveOut, &m_waveOutHdr, sizeof(WAVEHDR)))
    {
      // 에러 콘솔 출력
      cout << "waveOutPrepareHeader error" << endl;
      // 장치 닫기
      throw -1;
    }
    // 출력 시작
    if (waveOutWrite(m_hWaveOut, &m_waveOutHdr, sizeof(WAVEHDR)))
    {
      // 에러 콘솔 출력
      cout << "waveOutWrite error" << endl;
      // 장치 닫기
      throw -1;
    }
  }
}
// 서버용 Listen
void Wave::Listen(int port)
{
  WSADATA wsaData;
  // 소켓 실행.
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
  {
    return;
  }
  // Internet의 Stream 방식으로 소켓 생성
  SOCKET serverSock = socket(PF_INET, SOCK_STREAM, 0);
  // 소켓 주소 설정
  SOCKADDR_IN addr;
  // 구조체 초기화
  memset(&addr, 0, sizeof(addr));
  // 소켓은 Internet 타입
  addr.sin_family = AF_INET;
  // 서버이기 때문에 local 설정한다.
  // Any인 경우는 호스트를 127.0.0.1로 잡아도 되고 localhost로 잡아도 되고 양쪽 다 허용하게 할 수 있따. 그것이 INADDR_ANY이다.
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  // 서버 포트 설정...저는 9090으로 설정함.
  addr.sin_port = htons(port);
  // 설정된 소켓 정보를 소켓에 바인딩한다.
  if (bind(serverSock, (SOCKADDR *)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
  {
    // 에러 콘솔 출력
    cout << "error" << endl;
    return;
  }
  // 소켓을 대기 상태로 기다린다.
  if (listen(serverSock, SOMAXCONN) == SOCKET_ERROR)
  {
    // 에러 콘솔 출력
    cout << "error" << endl;
    return;
  }
  // 접속 설정 구조체
  int len = sizeof(SOCKADDR_IN);
  SOCKADDR_IN clientAddr;
  // client가 접속을 하면 SOCKET을 받는다.
  m_sock = accept(serverSock, (SOCKADDR *)&clientAddr, &len);
  // 소리 입출력 장치 초기화
  Init();
  // 접속 정보 콘솔에 출력
  cout << "Client connected IP address = " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << endl;
  // 소리 입력 시작
  if (waveInStart(m_hWaveIn))
  {
    // 에러 콘솔 출력
    cout << "waveInStart error" << endl;
    // 장치 닫기
    throw -1;
  }
  // 소켓 대기로 스피커 출력을 한다.
  Receive();
  // 장치 리소스 해제
  waveInClose(m_hWaveIn);
  waveOutClose(m_hWaveOut);
  // 소켓 해제
  closesocket(m_sock);
  closesocket(serverSock);
}
// 클라이언트용 접속 함수
void Wave::Connect(const char *ip, int port)
{
  // 소켓 정보 데이터 설정
  WSADATA wsaData;
  // 소켓 실행.
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
  {
    return;
  }
  // Internet의 Stream 방식으로 소켓 생성
  m_sock = socket(PF_INET, SOCK_STREAM, 0);
  // 소켓 주소 설정
  SOCKADDR_IN addr;
  // 구조체 초기화
  memset(&addr, 0, sizeof(addr));
  // 소켓은 Internet 타입
  addr.sin_family = AF_INET;
  // 접속 IP 설정
  addr.sin_addr.s_addr = inet_addr(ip);
  // 포트 설정
  addr.sin_port = htons(port);
  // 접속
  if (connect(m_sock, (SOCKADDR *)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
  {
    // 에러 콘솔 출력
    cout << "error" << endl;
    throw -1;
  }
  // 소리 입출력 장치 초기화
  Init();
  // 접속
  cout << "Server connected" << endl;
  // 소리 입력 시작
  if (waveInStart(m_hWaveIn))
  {
    // 에러 콘솔 출력
    cout << "waveInStart error" << endl;
    // 장치 닫기
    throw -1;
  }
  // 소켓 대기로 스피커 출력을 한다.
  Receive();
  waveInClose(m_hWaveIn);
  waveOutClose(m_hWaveOut);
  closesocket(m_sock);
}