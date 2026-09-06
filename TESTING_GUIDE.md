# Testing Guide

## Overview

Local-only testing for MHGame. Do **not** use production credentials or public IPs.

## Test accounts (local placeholders)

Set these yourself after DB setup — values below are placeholders only:

| Account | Password | Role | Notes |
|---------|----------|------|-------|
| admin | CHANGE_ME | GM | 管理测试 |
| test1 | CHANGE_ME | 普通 | 功能测试 |
| test2 | CHANGE_ME | 普通 | 压力测试 |

Login example: `admin / CHANGE_ME`（根据实际本地配置）

## Ports (localhost)

| Service | Port |
|---------|------|
| Distribute | 16001 |
| Agent | 17001 |
| Map | 18001 |
| Monitoring | 20001 |

## Smoke checks

1. SQL: `sqlcmd -S .\SQLEXPRESS -U mhgame -P "CHANGE_ME_DB_PASSWORD" -Q "SELECT 1"`
2. Start servers via `start_system.ps1`
3. Client connect to `127.0.0.1`

## Notes

- Rotate any previously published test passwords; treat them as compromised.
- Keep monitoring topology on `127.0.0.1` in the public tree.
