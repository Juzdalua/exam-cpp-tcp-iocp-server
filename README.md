# TCP IOCP Server  

## Folders

| Name | Description |
| ------ | ------ |
| Binary | output exe files. |
| Libraries | libriry files. |
| GameServer | Main Server files. |
| CoreLib | GameServer Core files. |

## Brabch
| Name | Description |
| ------ | ------ |
| Listener-Session | IOCP 기본 테스트 환경. |
| AcceptEx-WSARecv | 비동기 환경 테스트. |
| Use-JobQueue | 패킷 지연으로 Lock 경합을 줄인 환경. |
| Add-GlobalSendQueue | 클라이언트에 송신할 패킷들을 따로 분리한 환경 |
| Connect-Unity | Stateless 서버 결합 및 쓰레드풀을 활용한 환경 |

#### Version 1.0
Add-GlobalSendQueue

#### Version 1.1
Connect-Unity 진행중..
