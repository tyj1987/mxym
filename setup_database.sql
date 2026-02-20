-- =============================================
-- MHGame Database Setup Script
-- =============================================
-- This script creates the necessary databases
-- and logins for the MHGame server system.
--
-- Prerequisites:
-- - SQL Server Express or full SQL Server installed
-- - SQL Server Management Studio (SSMS) or sqlcmd
-- - Database backup files in 数据库/ folder
-- =============================================

-- Execute this script as Administrator (sa)

-- =============================================
-- 1. Create Login
-- =============================================
USE master;
GO

-- Check if login exists, if not create it
IF NOT EXISTS (SELECT name FROM sys.server_principals WHERE name = 'mhgame')
BEGIN
    CREATE LOGIN mhgame WITH PASSWORD = 'MHGame@2026!';
END
GO

-- =============================================
-- 2. Create Databases
-- =============================================

-- MHCMEMBER Database (Account/Membership)
IF NOT EXISTS (SELECT name FROM sys.databases WHERE name = 'MHCMEMBER')
BEGIN
    CREATE DATABASE MHCMEMBER;
END
GO

-- MHGAME Database (Game Data)
IF NOT EXISTS (SELECT name FROM sys.databases WHERE name = 'MHGAME')
BEGIN
    CREATE DATABASE MHGAME;
END
GO

-- MHLOG Database (Logs)
IF NOT EXISTS (SELECT name FROM sys.databases WHERE name = 'MHLOG')
BEGIN
    CREATE DATABASE MHLOG;
END
GO

-- =============================================
-- 3. Create Users and Grant Permissions
-- =============================================

USE MHCMEMBER;
GO
IF NOT EXISTS (SELECT name FROM sys.database_principals WHERE name = 'mhgame')
BEGIN
    CREATE USER mhgame FOR LOGIN mhgame;
    ALTER ROLE db_owner ADD MEMBER mhgame;
END
GO

USE MHGAME;
GO
IF NOT EXISTS (SELECT name FROM sys.database_principals WHERE name = 'mhgame')
BEGIN
    CREATE USER mhgame FOR LOGIN mhgame;
    ALTER ROLE db_owner ADD MEMBER mhgame;
END
GO

USE MHLOG;
GO
IF NOT EXISTS (SELECT name FROM sys.database_principals WHERE name = 'mhgame')
BEGIN
    CREATE USER mhgame FOR LOGIN mhgame;
    ALTER ROLE db_owner ADD MEMBER mhgame;
END
GO

-- =============================================
-- 4. Enable Mixed Mode Authentication (if needed)
-- =============================================
-- This requires registry modification or SSMS
-- Run this in SSMS or use PowerShell:
--
-- EXEC xp_instance_regwrite N'HKEY_LOCAL_MACHINE',
--     N'Software\Microsoft\MSSQLServer\MSSQLServer',
--     N'LoginMode', REG_DWORD, 2;
--
-- Then restart SQL Server service

PRINT 'Database setup completed!';
PRINT '';
PRINT 'Next steps:';
PRINT '1. Restore databases from backup files:';
PRINT '   - 数据库/MHCMEMBER.bak -> MHCMEMBER';
PRINT '   - 数据库/MHGAME.bak -> MHGAME';
PRINT '   - 数据库/MHLOG.bak -> MHLOG';
PRINT '';
PRINT '2. Update server config files with:';
PRINT '   DBUser=mhgame';
PRINT '   DBPassword=MHGame@2026!';
PRINT '';
PRINT '3. Restart SQL Server service';
GO
