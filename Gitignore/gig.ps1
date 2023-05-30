param(
    [Parameter(Mandatory=$true)]
    [string[]]$list
)
$params = ($list | ForEach-Object { [uri]::EscapeDataString($_) }) -join ","
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
Invoke-WebRequest -Uri "https://www.gitignore.io/api/$params" | select -ExpandProperty content | Out-File -FilePath $(Join-Path -path $pwd -ChildPath ".gitignore") -Encoding ascii
