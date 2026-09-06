# MHGame Setup Guide

## Prerequisites
- Windows with SQL Server Express
- Visual Studio build tools (see BUILD_GUIDE.md)

## Database setup

1. Restore backups from a **private** location (do **not** commit `*.bak` or `数据库/`).
2. Create login (use a strong local password; never commit it):

```sql
CREATE LOGIN mhgame WITH PASSWORD = 'CHANGE_ME_DB_PASSWORD';
CREATE USER mhgame FOR LOGIN mhgame;
ALTER ROLE db_owner ADD MEMBER mhgame;
```

Or run `setup_database.sql` after replacing `CHANGE_ME_DB_PASSWORD`.

## Server config

Edit `Server/asInfo.ini`, `Server/masInfo.ini`, `Server/msInfo.ini`:

```ini
[Database]
DBIP=.\SQLEXPRESS
DBName=MHCMEMBER
DBUser=sa
DBPassword=CHANGE_ME
```

Copy from example values; keep real passwords out of git.

## Verify DB connectivity

```bat
sqlcmd -S .\SQLEXPRESS -U mhgame -P "CHANGE_ME_DB_PASSWORD" -Q "SELECT COUNT(*) FROM MHCMEMBER.dbo.USER_INFO"
```

## Start services

See `start_system.ps1` / `QUICK_REFERENCE.md`. Default local ports: Distribute 16001, Agent 17001, Map 18001, Monitoring 20001.

## Security notes

- Rotate any passwords that were previously committed to this public repo.
- Keep `*.bak` and real `DBPassword` values local-only (see `.gitignore`).
