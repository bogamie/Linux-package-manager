# Linux Package Manager TUI

<div align="center">

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Language](https://img.shields.io/badge/language-C-green.svg)
![Platform](https://img.shields.io/badge/platform-Linux-orange.svg)

**Debian/Ubuntu 터미널 패키지 매니저**

[소개](#소개) • [기능](#기능) • [설치](#설치) • [사용법](#사용법) • [스크린샷](#스크린샷) • [보안](#보안) • [아키텍처](#아키텍처) • [포트폴리오-취업-활용](#포트폴리오취업-활용)

</div>

---

## 📋 소개

Linux Package Manager TUI는 Debian/Ubuntu 계열 리눅스에서 시스템 패키지를 사용자 친화적이고 안전하게 관리할 수 있는 터미널 기반 UI 애플리케이션입니다. C와 ncurses로 구현되어 가볍고 직관적이며, Vim 스타일의 키 바인딩과 화이트리스트 기반 입력 검증으로 보안까지 고려했습니다.

주요 특징
- 가벼운 설계: C + `-O2` 최적화를 활용한 단순하고 안정적인 동작
- 보안 강화: 명령어 인젝션을 원천 차단하는 입력 검증
- 사용성: j/k, gg/G, /, i, d, u 등 Vim 친화적 단축키
- 구조적 설계: UI/로직/유틸 분리, SOLID 지향 모듈 설계

---

## ✨ 기능

### 핵심 기능
- 📦 패키지 관리: 설치/삭제, 상세 정보 보기, 시스템 업데이트/업그레이드
- 🔍 스마트 검색: apt 캐시 기반 검색과 필터링
- 🎨 현대적 TUI: 깔끔한 ncurses UI, 선택 강조, 터미널 크기 대응, 부드러운 페이지네이션

### 기술 하이라이트
- ⚙️ 효율적 빌드: `-O2` 최적화
- 🔒 보안 강화: 화이트리스트 기반 입력 검증으로 쉘 인젝션 차단
- 🧩 모듈 구조: UI/로직/유틸/상수 분리
- 💾 메모리 안정성: 중앙집중형 해제, 누수 방지
- 🛡️ 오류 처리: 포괄적인 예외/에러 경로 처리

---

## 🚀 설치

### 사전 준비 (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install build-essential libncurses5-dev libncursesw5-dev
gcc --version  # C11 지원 gcc 권장
```

### 빌드
```bash
git clone https://github.com/bogamie/Linux-package-manager.git
cd Linux-package-manager
make         # 릴리스 빌드 (최적화)
./package_manager       # 실행
```

### 빌드 옵션
```bash
make          # 표준 빌드 (-O2)
make debug    # 디버그 빌드 (-g)
make clean    # 산출물 정리
make rebuild  # 클린 후 빌드
make help     # 사용 가능한 명령 보기
```

---

## 💻 사용법

### 빠른 시작
```bash
./package_manager       # 실행
./package_manager -help # 도움말
./package_manager -U    # 시작 시 시스템 업데이트/업그레이드 수행 (sudo 필요)
```

### 키보드 단축키

| 키 | 동작 | 설명 |
|---|---|---|
| `j` | 아래로 | 다음 패키지 |
| `k` | 위로 | 이전 패키지 |
| `gg` | 맨 위 | 첫 항목으로 이동 |
| `G` | 맨 아래 | 마지막 항목으로 이동 |
| `Enter` | 상세 보기 | 선택 항목 정보 |
| `i` | 설치 | 선택 패키지 설치 |
| `d` | 삭제 | 선택 패키지 제거 |
| `u` | 업데이트 | 시스템 업데이트/업그레이드 |
| `/` | 검색 | 패키지 검색 |
| `q` | 종료 | 프로그램 종료 |

### 예시
설치
```
1) ./package_manager 실행 → 2) j/k로 항목 이동 → 3) 'i' → 4) sudo 비밀번호 입력
```

검색
```
1) '/' → 2) 패키지명 입력(예: "vim") → 3) j/k로 탐색 → 4) Enter로 상세 보기
```

### 실행 옵션

| 옵션 | 설명 |
|------|------|
| `-help`, `--help` | 도움말 출력 후 종료 |
| `-U`, `--auto-update` | 시작 직후 시스템 업데이트 & 업그레이드 수행 (sudo 비밀번호 필요) |

기본 실행(`./package_manager`)은 시작 시 sudo 비밀번호를 요구하지 않습니다. 설치/삭제(i/d) 또는 시스템 업데이트(u) 기능을 사용할 때만 sudo 권한이 필요합니다.

---

## 🖼️ 스크린샷

<div align="center">

![메인 목록 화면](screenshot/Screenshot%20from%202025-01-14%2015-43-08.png)

| 설치 | 삭제 | 상세 정보 |
|---|---|---|
| ![패키지 설치](screenshot/Screenshot%20from%202025-01-14%2015-47-03.png) | ![패키지 삭제](screenshot/Screenshot%20from%202025-01-14%2015-47-09.png) | ![Vim 키 예시](screenshot/Screenshot%20from%202025-01-14%2015-47-23.png) |

</div>

---

## 🔒 보안

화이트리스트 기반 입력 검증으로 쉘 메타문자와 리디렉션, 따옴표, 역슬래시 등을 차단합니다.

허용 문자
- 영숫자: `a-z`, `A-Z`, `0-9`
- 특수: `-`, `.`, `_`, `+`, `:`

차단 문자(예)
- 메타: `;`, `|`, `&`, `$`, `` ` ``
- 리디렉션: `<`, `>`, `>>`, `<<`
- 인용부호: `'`, `"`
- 괄호/역슬래시: `()`, `\`

길이 제한
- 패키지명 ≤ 255, 검색어 ≤ 256, 커맨드 버퍼 512

---

## 🏗️ 아키텍처

```
src/
├── main.c              # 진입점
├── package_manager.c   # 패키지 관리 로직 (dpkg/apt)
├── package_manager.h   # 인터페이스
├── ui.c                # ncurses 렌더링
├── ui.h                # UI 인터페이스
├── utils.c             # 유틸리티(검증/메모리 등)
├── utils.h             # 유틸 인터페이스
└── constants.h         # 상수 정의
```

설계 원칙
- 관심사 분리(SoC), SOLID 지향
- 에러 핸들링 경로 일관화, 안전한 메모리 해제

예시 자료구조
```c
typedef struct Package {
	char *name;        // 패키지명
	char *version;     // 버전
	char *description; // 설명
} Package;
```

---

## 🛠️ 기술 상세

컴파일 플래그
```makefile
CFLAGS = -Wall -Wextra -O2 -std=c11
```

의존성
- ncurses, dpkg-query, apt/apt-get, apt-cache

메모리/오류 처리
- 중앙집중 해제(`freePackages`), 안전 문자열 복제, NULL-세이프 해제
- `popen/pclose` 실패 처리, 터미널 크기 검증, 안정적인 예외 처리

---

## 📊 동작 특성

- 동작 성능은 하드웨어, 네트워크, 설치된 패키지 수 등에 따라 달라질 수 있습니다.
- 검색 기능은 apt/apt-cache 호출 특성상 환경에 따라 느리게 느껴질 수 있습니다.
- 필요 시 비동기 검색, 캐시 고도화 등으로 개선 여지가 있습니다.

---

## 🧪 테스트

수동 테스트 체크리스트
- [x] 목록/탐색, 설치/삭제(sudo 필요)
- [x] 시스템 업데이트
- [x] 유효/무효 입력 검색(보안 검증)
- [x] 터미널 리사이즈 처리
- [x] 메모리 누수 점검(valgrind)

보안 테스트 예
```bash
# 예: test; rm -rf /tmp/test #  → 입력 거부가 되어야 함
```

---

## 📝 변경 로그

### 1.1.0 (2025-11-13)
- ✅ 종합 입력 검증 및 보안 강화
- ✅ constants.h 기반 모듈 구조 정리
- ✅ 메모리 관리 강화 및 오류 처리 개선
- ✅ 'u' 키 시스템 업데이트 추가
- ✅ 컴파일 최적화 개선

### 1.0.0
- 초기 릴리스: 기본 패키지 관리, Vim 스타일 내비게이션, 검색

---

## 📄 라이선스

MIT License (c) 2025 bogamie

---

## 👨‍💻 작성자

**bogamie**
- GitHub: [@bogamie](https://github.com/bogamie)
- Project: [Linux-package-manager](https://github.com/bogamie/Linux-package-manager)

---

## 💼 포트폴리오/취업 활용

이 프로젝트로 어필할 수 있는 역량
- C(C11) 시스템 프로그래밍, 터미널 UI(ncurses) 구현
- Linux 패키지 생태계 이해(apt, dpkg-query)
- 보안 중심 개발(입력 검증, 경계 체크, 인젝션 차단)
- 메모리 관리, `-Wall -Wextra -O2` 무경고 빌드
- 모듈형 아키텍처, 견고한 에러 핸들링, 테스트 관행

이력서 Bullet 예시
- "C와 ncurses로 Debian/Ubuntu용 TUI 패키지 매니저 구현. 화이트리스트 기반 입력 검증으로 커맨드 인젝션 원천 차단, UI/로직/유틸 모듈화와 일관된 에러/메모리 관리로 안정성 및 유지보수성 확보."

면접 포인트
- 화이트리스트 검증 설계 배경과 허용/차단 기준
- `popen/pclose` 오류/자원 관리 전략
- 대량 리스트에서의 스크롤/검색 성능 설계
- 모듈 경계와 변경 전파 최소화 방법

---