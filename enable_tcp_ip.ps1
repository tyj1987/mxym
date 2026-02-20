# Enable TCP/IP for SQL Server Express
$null = [reflection.assembly]::loadwithpartialname("Microsoft.SqlServer.Smo")
$null = [reflection.assembly]::loadwithpartialname("Microsoft.SqlServer.SqlWmiManagement")

$mc = New-Object Microsoft.SqlServer.Management.Smo.Wmi.ManagedComputer
$tcp = $mc.ServerInstances["SQLEXPRESS"].ServerProtocols["Tcp"]
$tcp.IsEnabled = $true
$tcp.Alter()

Write-Host "TCP/IP protocol enabled for SQLEXPRESS"
Write-Host "Please restart SQL Server service"
