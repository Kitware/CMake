# https://learn.microsoft.com/en-us/windows/win32/msi/database-tables

param (
  $file
  )

function printTable {
  param (
    $msi,
    [string]$name,
    [int[]]$columns = (1),
    [string]$where = ""
    )

  try {
    $view = $msi.OpenView("select * from " + $name + $where)
    $view.Execute()
    while ($record = $view.Fetch()) {
      Write-Host ($name + ": " + ($columns | ForEach-Object {"'" + $record.StringData($_) + "'"}))
    }
  } catch {}
}

$installer = New-Object -ComObject WindowsInstaller.Installer
$msi = $installer.OpenDatabase($file, 0)

printTable -msi $msi -name "Component" -columns 1,3
printTable -msi $msi -name "Directory" -columns 1,2,3
printTable -msi $msi -name "File" -columns 1,2,3
printTable -msi $msi -name "Shortcut" -columns 1,2,3,4
printTable -msi $msi -name "Registry" -columns 6,2
printTable -msi $msi -name "Property" -columns 1,2 -where " where Property = 'ALLUSERS' or Property = 'MSIINSTALLPERUSER'"
printTable -msi $msi -name "LaunchCondition" -columns 1,2
