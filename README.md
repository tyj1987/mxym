# MHGame - Korean MMORPG Game System

A large-scale Korean MMORPG game development project with client, server, and game library components.

## Quick Start

### Prerequisites

- **Visual Studio 2022** - For compilation
- **DirectX SDK June 2010** - Graphics rendering
- **Windows 10/11 SDK** - Platform SDK
- **SQL Server Express** (optional) - For server database

### 1. Build the Project

```powershell
# Build all modules in correct order
.\build_all.ps1 -All

# Or build specific phases
.\build_all.ps1 -Phase 1  # Base libraries
.\build_all.ps1 -Phase 4  # Servers
```

### 2. Verify Build

```powershell
.\verify_system.ps1
```

### 3. Start the System

```powershell
# Start all servers and client
.\start_system.ps1

# Start servers only
.\start_system.ps1 -SkipClient

# Stop all servers
.\stop_system.ps1
```

### 4. Manual Server Startup (Alternative)

```batch
cd Server
StartServers.bat
```

## Project Structure

```
D:\mxym\
├── [CC]Header/          # Shared header files
├── [CC]Ability/         # Ability system
├── [CC]Skill/           # Skill system
├── [CC]BattleSystem/    # Battle system
├── [CC]Quest/           # Quest system
├── [Lib]YHLibrary/      # Core library
├── [Lib]BaseNetwork/    # Network library
├── [Server]Map/         # Map server
├── [Server]Agent/       # Agent server
├── [Client]MH/          # Client source
├── Server/              # Server deployment
├── PlayDH/              # Client deployment
└── Database/            # Database files
```

## Server Architecture

| Server | Port | Description |
|--------|------|-------------|
| MonitoringServer | 20001 | Monitoring agent |
| AgentServer | 17001 | Client connections |
| MapServer | 18001 | Game logic |
| DistributeServer | 16001 | Server communication (optional) |

**Startup Order**: MonitoringServer → AgentServer → MapServer → DistributeServer

## Build Order

```
Layer 1: Base Static Libraries
  └── YHLibrary, HSEL, ZipArchive

Layer 2: Base DLLs
  └── BaseNetwork, DBThread

Layer 3: Game Core Libraries
  └── Ability, Skill, BattleSystem, Quest, Suryun

Layer 4: Server Applications
  └── Monitoring, Agent, Map, Distribute

Layer 5: Client Application
  └── MHClient
```

## Documentation

| Document | Description |
|----------|-------------|
| [CLAUDE.md](CLAUDE.md) | Main development guide |
| [QUICK_REFERENCE.md](QUICK_REFERENCE.md) | Command reference |
| [BUILD_GUIDE.md](BUILD_GUIDE.md) | Detailed build instructions |
| [TROUBLESHOOTING.md](TROUBLESHOOTING.md) | Error resolution |
| [TESTING_GUIDE.md](TESTING_GUIDE.md) | Testing procedures |
| [REGRESSION_TEST.md](REGRESSION_TEST.md) | Regression testing workflow |

## Test Accounts

| Account | Password | GM Level | Description |
|---------|----------|----------|-------------|
| admin | admin123 | 9 | Super admin |
| gm | gm123 | 5 | GM assistant |
| test | test123 | 0 | Regular player |

## Configuration Files

- `Server/masInfo.ini` - Monitoring agent config
- `Server/msInfo.ini` - Monitoring server config
- `Server/ServerInfo.ini` - Server information
- `Server/serverset/serverset.txt` - Server set number
- `PlayDH/server.txt` - Client server config

## Troubleshooting

### Build Errors

- Ensure DirectX SDK is installed and `DXSDK_DIR` is set
- Build modules in correct dependency order
- Check library paths in project settings

### Server Won't Start

- Check if ports are already in use: `netstat -an | findstr "17001"`
- Check log files in `Server/Log/`
- Verify database connection in config files

### Client Can't Connect

- Verify servers are running: `tasklist | findstr "Server"`
- Check `PlayDH/server.txt` has correct IP
- Check firewall settings

## Development Notes

### Coding Style

- Hungarian notation: `m_` prefix for members, `p` for pointers
- Singleton pattern: `USINGTON(ClassName)->Method()`
- Memory management: `SAFE_DELETE(ptr)`, `SAFE_RELEASE(pInterface)`

### Key Macros

```cpp
USINGTON(ClassName)->Method();  // Singleton access
GAMERESRCMNGR->LoadData();      // Resource manager
SAFE_DELETE(ptr);               // Safe delete
ASSERT(condition);              // Assertion
```

## Modernization

The project is being modernized:
- DirectX 8 → DirectX 12 migration (`4DYUCHIGX_RENDER_D3D12/`)
- VS2008 → VS2022 compatibility
- C++17 standard

## License

Private project for educational purposes.

---

**Last Updated**: 2026-02-20
**Status**: Active development
