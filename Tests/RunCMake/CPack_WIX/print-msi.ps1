# https://learn.microsoft.com/en-us/windows/win32/msi/database-tables

param (
  $file
  )

function printTable {
  param (
    $msi,
    [string]$name,
    [int[]]$columns = (1)
    )

  try {
    $view = $msi.OpenView("select * from " + $name)
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
