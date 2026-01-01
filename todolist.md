## TODO

### APP (로컬)
1. Serial 통신 구성
   - [ ] 시리얼 통신 정보 저장 하기
     - 해당 포트 사용/미사용
     - 데이터 비트
     - 스탑 비트
     - 패리티
     - 가칭
     - 통신 장치 주소
     - 기능 코드
     - 통신 주소 (데이터 길이 1 고정)
     - 데이터 스캔 인터벌
     - 타임 아웃
     - 리트라이 횟수
   - [ ] 통신 Open
   - [ ] 통신 Close
   - [ ] 통신 포트 생성, 제거, 편집
   - [ ] 통신 실패 시 에러 처리
     - 타임 아웃
     - CRC ERROR
     - No response
     - insufficient byte
   - [ ] 통신 설정 변경 시 다음 세션부터 적용

2. 로스팅 데이터 처리
   - [ ] 열려있는 통신 포트 확인
     - [ ] 통신정보=>사용/미사용 사용 통신 포트 연결
     - [ ] 연결 불가능 시 통신정보=>사용/미사용 미사용 처리
   - [ ] 시작 시 설정에 있는 인터벌로 데이터 통신 개시 및 서버에 세션 생성 요청
     - 로그인이 되어있지 않은 경우 이전 데이터 삭제 후 재 기록
   - [ ] 동작 중 로컬 및 서버에 데이터 기록
     - 시리얼 통신은 매 시리얼통신 인터벌
     - 서버 통신은 5초에 한번
     - 네트워크 실패 시 로컬에 임시 저장
     - 네트워크 복구 시 재전송 (서버통신 인터벌 상관 없이)
     - 로그인이 되어있지 않은 경우 로컬에만 기록 (기록 인터벌 정해야 함 1~5초?)
   - [ ] 종료 시 서버에 종료 요청
     - 로그인이 되어있지 않은 경우 로컬에 저장 (최대 저장 갯수 제한)
   - [ ] 세션 데이터
     - 세션 이름 (디폴트 현재 날짜 시각)
     - 유저 코드
     - 디바이스 코드
     - 그램 수
     - 노트 (특이사항)
     - 에러 코드
     - 시작 시간 (세션 시작 시간)
     - 종료 시간 (세션 종료 시간)
   - [ ] 로스팅 데이터 형식
     - 세션 코드
     - 통신 데이터 (시간, 온도)
     - 통신 시간 (현재 시간 - 시작 시간)

3. 플롯 (UI)
   - [ ] 세션 히스토리 리스트 DB에서 불러오기
   - [ ] 데이터 플롯 하기
   - [ ] 현재 세션 플롯 하기
   - [ ] 세션 시작, 종료 표시
   - [ ] 세션 경과 시간 표시
   - [ ] 에러 발생 시 시각화 필요
     - 시리얼 통신
     - 네트워크 통신
     - 로그인 / 비 로그인 구분

4. 기타
   - [ ] 로그인 인증 
   - [ ] 로그인 상태 유지
   - [ ] 하트비트
     - On Off 상태 확인
     - 디바이스 코드
     - 세션 코드
     - 에러 코드
   - [ ] 특정 세션 엑셀 저장


### 서버
1. 사용자 관리
   - [ ] 회원가입
   - [ ] 로그인
   - [ ] JWT or Session
   - [ ] 사용자 ID
2. 장비 관리
   - [ ] 디바이스 연결
   - [ ] 디바이스 삭제
   - [ ] 수신 상태 (하트비트)
3. DB
   - [ ] 테이블
     - 세션 데이터
     - 로스팅 데이터
     - 시리얼 통신 데이터
     - 이벤트 / 상태 데이터
   - [ ] 데이터 조회
     - 각 세션 별 데이터 조회 필요
4. API
   - [ ] 세션 생성
   - [ ] 세션 종료
   - [ ] 데이터 수신
   - [ ] 수집 상태 조회 (heatbeat)


## 작업
### 우선순위 재정렬 (추천)
#### 1단계 (MVP)
- Serial 통신
- 세션 생성/종료
- 데이터 수집 → 서버 저장
- 세션별 조회
#### 2단계
- 실시간 플롯
- 하트비트
- 장비 상태 UI
#### 3단계
- raw 데이터 옵션
- 재전송 / 복구
- 통계 / 비교


## DB 스키마 설계
### 구조
```
users 1 ─── N devices
devices 1 ─── N sessions
sessions 1 ─── N roasting_samples
sessions 1 ─── N events
devices 1 ─── N serial_configs
devices 1 ─── N device_heartbeats
```
### 인덱스
```
INDEX sessions(user_id, start_time)
INDEX roasting_samples(session_id, sample_index)
INDEX device_heartbeats(device_id, reported_at)
INDEX devices(user_id)
```
### 사용자
```
users
-----
id              BIGINT PK
email           VARCHAR UNIQUE
password_hash   VARCHAR
created_at      TIMESTAMP
last_login_at   TIMESTAMP
```
### 장비
```
devices
-------
id              BIGINT PK
user_id         BIGINT FK -> users.id
device_code     VARCHAR UNIQUE
device_name     VARCHAR
device_token    VARCHAR UNIQUE
created_at      TIMESTAMP
last_seen_at    TIMESTAMP
```
### 시리얼 포트
```
serial_configs
--------------
id                  BIGINT PK
device_id           BIGINT FK -> devices.id
port_name           VARCHAR
baud_rate           INT
data_bits           INT
stop_bits           INT
parity              VARCHAR
slave_address       INT
function_code       INT
register_address    INT
scan_interval_ms    INT
timeout_ms          INT
retry_count         INT
is_enabled          BOOLEAN
updated_at          TIMESTAMP
```
### 세션
```
sessions
--------
id              BIGINT PK
session_code    VARCHAR UNIQUE
user_id         BIGINT FK -> users.id
device_id       BIGINT FK -> devices.id
session_name    VARCHAR
grams           INT
note            TEXT
start_time      TIMESTAMP
end_time        TIMESTAMP
status          VARCHAR   -- RUNNING / FINISHED / ERROR
created_at      TIMESTAMP
```
### 로스팅 데이터
```
roasting_samples
----------------
id              BIGINT PK
session_id      BIGINT FK -> sessions.id
sample_index    INT
elapsed_ms      INT
temperature     FLOAT
error_code      VARCHAR
created_at      TIMESTAMP
```
### 하트비트
```
device_heartbeats
-----------------
id              BIGINT PK
device_id       BIGINT FK -> devices.id
session_id      BIGINT FK -> sessions.id NULL
status          VARCHAR   -- ONLINE / OFFLINE / RUNNING
error_code      VARCHAR
reported_at     TIMESTAMP
```
### 이벤트
```
events
------
id              BIGINT PK
device_id       BIGINT FK -> devices.id
session_id      BIGINT FK -> sessions.id NULL
event_type      VARCHAR   -- SESSION_START, COMM_ERROR, SESSION_END
message         TEXT
created_at      TIMESTAMP
```