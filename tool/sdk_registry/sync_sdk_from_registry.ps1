[CmdletBinding()]
param(
    [string]$ConfigPath = "",
    [string]$SdkBase = "",
    [string]$Board = "",
    [int]$NetworkTimeoutSeconds = 5,
    [switch]$Force
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Write-Info {
    param([string]$Message)
    Write-Host "[sdk-sync] $Message"
}

function Write-StepProgress {
    param(
        [string]$Stage,
        [int]$Index,
        [int]$Total,
        [string]$Item = ""
    )
    if ($Total -le 0) {
        Write-Info "$Stage"
        return
    }
    $percent = [Math]::Floor(($Index * 100.0) / $Total)
    if ([string]::IsNullOrWhiteSpace($Item)) {
        Write-Info "$Stage [$Index/$Total] ($percent`%)"
    } else {
        Write-Info "$Stage [$Index/$Total] ($percent`%): $Item"
    }
}

function Format-Bytes {
    param([Int64]$Bytes)
    if ($Bytes -ge 1GB) { return ("{0:N2} GB" -f ($Bytes / 1GB)) }
    if ($Bytes -ge 1MB) { return ("{0:N2} MB" -f ($Bytes / 1MB)) }
    if ($Bytes -ge 1KB) { return ("{0:N2} KB" -f ($Bytes / 1KB)) }
    return ("{0} B" -f $Bytes)
}

function Get-JsonValue {
    param(
        [Parameter(Mandatory = $true)] [object]$Obj,
        [Parameter(Mandatory = $true)] [string]$Name
    )
    if ($null -eq $Obj) { return $null }
    $prop = $Obj.PSObject.Properties[$Name]
    if ($null -eq $prop) { return $null }
    return $prop.Value
}

function Get-ErrorMessage {
    param([object]$Err)
    if ($null -eq $Err) { return "unknown error" }
    if ($Err.Exception -and -not [string]::IsNullOrWhiteSpace($Err.Exception.Message)) {
        return [string]$Err.Exception.Message
    }
    $txt = ($Err | Out-String).Trim()
    if ([string]::IsNullOrWhiteSpace($txt)) { return "unknown error" }
    return $txt
}

function Get-AuthHeaders {
    param([string]$Token)
    $headers = @{
        "Accept" = "application/json"
    }
    if ($env:REGISTRY_JOB_TOKEN) {
        $headers["JOB-TOKEN"] = $env:REGISTRY_JOB_TOKEN
    } else {
        $headers["PRIVATE-TOKEN"] = $Token
    }
    return $headers
}

function As-Array {
    param([object]$Value)
    if ($null -eq $Value) { return @() }
    return @($Value)
}

function Invoke-RestGetWithRetry {
    param(
        [string]$Uri,
        [hashtable]$Headers,
        [int]$TimeoutSeconds = 5,
        [int]$MaxAttempts = 2,
        [int]$SleepSeconds = 1
    )
    for ($i = 1; $i -le $MaxAttempts; $i++) {
        try {
            return Invoke-RestMethod -Method GET -Uri $Uri -Headers $Headers -TimeoutSec $TimeoutSeconds
        } catch {
            if ($i -eq $MaxAttempts) { throw }
            Start-Sleep -Seconds $SleepSeconds
        }
    }
}

function Download-FileWithRetry {
    param(
        [string]$Uri,
        [hashtable]$Headers,
        [string]$OutFile,
        [string]$Label = "",
        [switch]$ShowProgress,
        [int]$TimeoutSeconds = 5,
        [int]$MaxAttempts = 2,
        [int]$SleepSeconds = 1
    )
    for ($i = 1; $i -le $MaxAttempts; $i++) {
        try {
            $response = $null
            $src = $null
            $dst = $null

            $request = [System.Net.HttpWebRequest]::Create($Uri)
            $request.Method = "GET"
            $request.Timeout = $TimeoutSeconds * 1000
            $request.ReadWriteTimeout = $TimeoutSeconds * 1000
            foreach ($k in @($Headers.Keys)) {
                $key = [string]$k
                $val = [string]$Headers[$k]
                if ($key -ieq "Accept") {
                    $request.Accept = $val
                } else {
                    $request.Headers[$key] = $val
                }
            }

            $response = [System.Net.HttpWebResponse]$request.GetResponse()
            $total = [Int64]$response.ContentLength

            $src = $response.GetResponseStream()
            $dst = [System.IO.File]::Open($OutFile, [System.IO.FileMode]::Create, [System.IO.FileAccess]::Write, [System.IO.FileShare]::None)
            $buf = New-Object byte[] (256KB)
            $downloaded = [Int64]0
            $lastPercent = -1
            $lastLog = [DateTime]::UtcNow.AddSeconds(-10)

            while (($read = $src.Read($buf, 0, $buf.Length)) -gt 0) {
                $dst.Write($buf, 0, $read)
                $downloaded += [Int64]$read

                if ($ShowProgress) {
                    $now = [DateTime]::UtcNow
                    if ($total -gt 0) {
                        $percent = [int][Math]::Floor(($downloaded * 100.0) / $total)
                        if (($percent -ge ($lastPercent + 5)) -or (($now - $lastLog).TotalSeconds -ge 1)) {
                            $prefix = if ([string]::IsNullOrWhiteSpace($Label)) { "Download progress" } else { "Download progress [$Label]" }
                            Write-Info "${prefix}: $percent% ($(Format-Bytes -Bytes $downloaded)/$(Format-Bytes -Bytes $total))"
                            $lastPercent = $percent
                            $lastLog = $now
                        }
                    } else {
                        if (($now - $lastLog).TotalSeconds -ge 1) {
                            $prefix = if ([string]::IsNullOrWhiteSpace($Label)) { "Download progress" } else { "Download progress [$Label]" }
                            Write-Info "${prefix}: $(Format-Bytes -Bytes $downloaded)"
                            $lastLog = $now
                        }
                    }
                }
            }

            if ($ShowProgress) {
                $prefix = if ([string]::IsNullOrWhiteSpace($Label)) { "Download progress" } else { "Download progress [$Label]" }
                if ($lastPercent -lt 100) {
                    Write-Info "${prefix}: 100%"
                }
            }
            return
        } catch {
            if (Test-Path -LiteralPath $OutFile) {
                Remove-Item -LiteralPath $OutFile -Force
            }
            if ($i -eq $MaxAttempts) { throw }
            $show = if ([string]::IsNullOrWhiteSpace($Label)) { $Uri } else { $Label }
            $reason = Get-ErrorMessage -Err $_
            Write-Info "Download failed, retry $i/${MaxAttempts}: $show, reason=$reason"
            Start-Sleep -Seconds $SleepSeconds
        } finally {
            if ($null -ne $dst) { $dst.Dispose() }
            if ($null -ne $src) { $src.Dispose() }
            if ($null -ne $response) { $response.Dispose() }
        }
    }
}

function Invoke-HeadWithRetry {
    param(
        [string]$Uri,
        [hashtable]$Headers,
        [int]$TimeoutSeconds = 5,
        [int]$MaxAttempts = 2,
        [int]$SleepSeconds = 1
    )
    for ($i = 1; $i -le $MaxAttempts; $i++) {
        try {
            Invoke-WebRequest -Method HEAD -Uri $Uri -Headers $Headers -TimeoutSec $TimeoutSeconds | Out-Null
            return
        } catch {
            if ($i -eq $MaxAttempts) { throw }
            Start-Sleep -Seconds $SleepSeconds
        }
    }
}

function Ensure-Dir {
    param([string]$Path)
    if (-not (Test-Path -LiteralPath $Path)) {
        New-Item -Path $Path -ItemType Directory | Out-Null
    }
}

function Get-ProjectPath {
    param([string]$ProjectIdOrPath)
    if ($ProjectIdOrPath -match '^\d+$') {
        return $ProjectIdOrPath
    }
    return [System.Uri]::EscapeDataString($ProjectIdOrPath)
}

function Get-PackageFileUrl {
    param(
        [string]$ApiV4Url,
        [string]$ProjectIdOrPath,
        [string]$PackageName,
        [string]$PackageVersion,
        [string]$FileName
    )
    $projectPath = Get-ProjectPath -ProjectIdOrPath $ProjectIdOrPath
    $pkg = [System.Uri]::EscapeDataString($PackageName)
    $ver = [System.Uri]::EscapeDataString($PackageVersion)
    $name = [System.Uri]::EscapeDataString($FileName)
    return "$ApiV4Url/projects/$projectPath/packages/generic/$pkg/$ver/$name"
}

function Test-PackageComplete {
    param(
        [string]$ApiV4Url,
        [string]$ProjectIdOrPath,
        [string]$PackageName,
        [string]$PackageVersion,
        [hashtable]$Headers,
        [string[]]$ExpectedFiles,
        [string]$ManifestFile,
        [int]$TimeoutSeconds = 5
    )

    $tempDir = Join-Path $env:TEMP ("vendor_sdk_probe_" + [Guid]::NewGuid().ToString("N"))
    Ensure-Dir -Path $tempDir
    try {
        try {
            $manifestPath = Join-Path $tempDir $ManifestFile
            $manifestUrl = Get-PackageFileUrl -ApiV4Url $ApiV4Url -ProjectIdOrPath $ProjectIdOrPath -PackageName $PackageName -PackageVersion $PackageVersion -FileName $ManifestFile
            Download-FileWithRetry -Uri $manifestUrl -Headers $Headers -OutFile $manifestPath -TimeoutSeconds $TimeoutSeconds
            $manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
            $names = @{}
            if ($manifest.files) {
                foreach ($entry in $manifest.files) {
                    if ($entry.name) { $names[[string]$entry.name] = $true }
                }
            }
            $allInManifest = $true
            foreach ($f in $ExpectedFiles) {
                if (-not $names.ContainsKey($f)) { $allInManifest = $false; break }
            }
            if ($allInManifest) { return $true }
        } catch {
            # no manifest or invalid manifest -> fallback probe below
        }

        foreach ($f in $ExpectedFiles) {
            $url = Get-PackageFileUrl -ApiV4Url $ApiV4Url -ProjectIdOrPath $ProjectIdOrPath -PackageName $PackageName -PackageVersion $PackageVersion -FileName $f
            try {
                Invoke-HeadWithRetry -Uri $url -Headers $Headers -TimeoutSeconds $TimeoutSeconds
            } catch {
                return $false
            }
        }
        return $true
    } finally {
        if (Test-Path -LiteralPath $tempDir) {
            Remove-Item -LiteralPath $tempDir -Recurse -Force
        }
    }
}

try {
    if ([string]::IsNullOrWhiteSpace($ConfigPath)) {
        $ConfigPath = Join-Path $PSScriptRoot "registry.config.json"
    }
    if ([string]::IsNullOrWhiteSpace($SdkBase)) {
        $SdkBase = Join-Path (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path "sdk"
    }

    if (-not (Test-Path -LiteralPath $ConfigPath)) {
        Write-Info "Config not found, skip sync: $ConfigPath"
        exit 0
    }

    $config = Get-Content -LiteralPath $ConfigPath -Raw | ConvertFrom-Json
    $apiV4Url = Get-JsonValue -Obj $config -Name "api_v4_url"
    $projectId = Get-JsonValue -Obj $config -Name "project_id"
    $packageName = Get-JsonValue -Obj $config -Name "package_name"
    $manifestFile = Get-JsonValue -Obj $config -Name "manifest_file"
    $tokenFromConfig = Get-JsonValue -Obj $config -Name "token"
    $boards = Get-JsonValue -Obj $config -Name "boards"

    if ([string]::IsNullOrWhiteSpace($apiV4Url)) { $apiV4Url = $env:REGISTRY_API_V4_URL }
    if ([string]::IsNullOrWhiteSpace($projectId)) { $projectId = $env:REGISTRY_PROJECT_ID }
    if ([string]::IsNullOrWhiteSpace($packageName)) { $packageName = $env:REGISTRY_PACKAGE_NAME }
    if ([string]::IsNullOrWhiteSpace($manifestFile)) { $manifestFile = "manifest.json" }
    $token = if ($env:REGISTRY_TOKEN) { $env:REGISTRY_TOKEN } else { $tokenFromConfig }

    if ([string]::IsNullOrWhiteSpace($apiV4Url) -or
        [string]::IsNullOrWhiteSpace($projectId) -or
        [string]::IsNullOrWhiteSpace($packageName)) {
        Write-Info "Config incomplete, skip sync."
        exit 0
    }
    if ([string]::IsNullOrWhiteSpace($token)) {
        Write-Info "Token not configured, skip sync."
        exit 0
    }

    $apiV4Url = $apiV4Url.TrimEnd("/")

    $boards = As-Array -Value $boards
    if (@($boards).Count -eq 0) {
        $boards = @(
            "board6e_ecat_cfm",
            "board6e_mc16",
            "board6750_ecat",
            "board6e_cfm",
            "board6e_p02"
        )
    }

    $targetBoards = @()
    if ([string]::IsNullOrWhiteSpace($Board)) {
        $targetBoards = @($boards)
    } else {
        $targetBoards = @($Board)
    }

    $expectedFiles = @()
    foreach ($board in @($targetBoards)) {
        $expectedFiles += "libvendor_sdk_$board.a"
    }

    $libDir = Join-Path $SdkBase "lib"
    Ensure-Dir -Path $libDir
    $statePath = Join-Path $libDir ".vendor_sdk_registry_state.json"

    $localVersion = $null
    $fileVersions = @{}
    if (Test-Path -LiteralPath $statePath) {
        try {
            $state = Get-Content -LiteralPath $statePath -Raw | ConvertFrom-Json
            $localVersion = Get-JsonValue -Obj $state -Name "version"
            $fileVersionsObj = Get-JsonValue -Obj $state -Name "file_versions"
            if ($null -ne $fileVersionsObj) {
                foreach ($prop in $fileVersionsObj.PSObject.Properties) {
                    if (-not [string]::IsNullOrWhiteSpace([string]$prop.Name) -and
                        -not [string]::IsNullOrWhiteSpace([string]$prop.Value)) {
                        $fileVersions[[string]$prop.Name] = [string]$prop.Value
                    }
                }
            }
            $stateFiles = As-Array -Value (Get-JsonValue -Obj $state -Name "files")
            if (-not [string]::IsNullOrWhiteSpace($localVersion)) {
                foreach ($sf in @($stateFiles)) {
                    if (-not [string]::IsNullOrWhiteSpace([string]$sf) -and
                        -not $fileVersions.ContainsKey([string]$sf)) {
                        $fileVersions[[string]$sf] = [string]$localVersion
                    }
                }
            }
        } catch {
            $localVersion = $null
            $fileVersions = @{}
        }
    }

    $allLocalExists = $true
    foreach ($file in @($expectedFiles)) {
        if (-not (Test-Path -LiteralPath (Join-Path $libDir $file))) {
            $allLocalExists = $false
            break
        }
    }

    $headers = Get-AuthHeaders -Token $token
    $projectPath = Get-ProjectPath -ProjectIdOrPath $projectId
    $queryPackage = [System.Uri]::EscapeDataString($packageName)
    $listUrl = "$apiV4Url/projects/$projectPath/packages?package_type=generic&package_name=$queryPackage&order_by=created_at&sort=desc&per_page=100"

    try {
        $pkgList = As-Array -Value (Invoke-RestGetWithRetry -Uri $listUrl -Headers $headers -TimeoutSeconds $NetworkTimeoutSeconds)
    } catch {
        if (-not $Force -and $allLocalExists) {
            $reason = Get-ErrorMessage -Err $_
            Write-Info "Registry unavailable, use local SDK libraries. scope=$Board local=$localVersion reason=$reason"
            exit 0
        }
        throw
    }
    if (@($pkgList).Count -eq 0) {
        throw "No generic package found for package_name=$packageName"
    }

    $candidateVersions = @()
    foreach ($pkg in @($pkgList)) {
        $ver = [string](Get-JsonValue -Obj $pkg -Name "version")
        if (-not [string]::IsNullOrWhiteSpace($ver)) {
            $candidateVersions += $ver
        }
    }
    $candidateVersions = As-Array -Value ($candidateVersions | Select-Object -Unique)
    if (@($candidateVersions).Count -eq 0) {
        throw "No package version found."
    }

    $latestVersion = $null
    $candidateCount = @($candidateVersions).Count
    $candidateIndex = 0
    foreach ($ver in @($candidateVersions)) {
        $candidateIndex++
        Write-StepProgress -Stage "Check package version" -Index $candidateIndex -Total $candidateCount -Item $ver
        if (Test-PackageComplete -ApiV4Url $apiV4Url -ProjectIdOrPath $projectId -PackageName $packageName -PackageVersion $ver -Headers $headers -ExpectedFiles $expectedFiles -ManifestFile $manifestFile -TimeoutSeconds $NetworkTimeoutSeconds) {
            $latestVersion = $ver
            break
        }
    }
    if ([string]::IsNullOrWhiteSpace($latestVersion)) {
        if (-not $Force -and $allLocalExists) {
            Write-Info "No reachable complete package version, use local SDK libraries. scope=$Board local=$localVersion"
            exit 0
        }
        throw "No complete package version found in registry."
    }

    $libDir = Join-Path $SdkBase "lib"
    Ensure-Dir -Path $libDir
    $statePath = Join-Path $libDir ".vendor_sdk_registry_state.json"

    $localVersion = $null
    $fileVersions = @{}
    if (Test-Path -LiteralPath $statePath) {
        try {
            $state = Get-Content -LiteralPath $statePath -Raw | ConvertFrom-Json
            $localVersion = Get-JsonValue -Obj $state -Name "version"
            $fileVersionsObj = Get-JsonValue -Obj $state -Name "file_versions"
            if ($null -ne $fileVersionsObj) {
                foreach ($prop in $fileVersionsObj.PSObject.Properties) {
                    if (-not [string]::IsNullOrWhiteSpace([string]$prop.Name) -and
                        -not [string]::IsNullOrWhiteSpace([string]$prop.Value)) {
                        $fileVersions[[string]$prop.Name] = [string]$prop.Value
                    }
                }
            }
            $stateFiles = As-Array -Value (Get-JsonValue -Obj $state -Name "files")
            if (-not [string]::IsNullOrWhiteSpace($localVersion)) {
                foreach ($sf in @($stateFiles)) {
                    if (-not [string]::IsNullOrWhiteSpace([string]$sf) -and
                        -not $fileVersions.ContainsKey([string]$sf)) {
                        $fileVersions[[string]$sf] = [string]$localVersion
                    }
                }
            }
        } catch {
            $localVersion = $null
            $fileVersions = @{}
        }
    }

    $allLocalUpToDate = $true
    foreach ($file in @($expectedFiles)) {
        $localFilePath = Join-Path $libDir $file
        if (-not (Test-Path -LiteralPath $localFilePath)) {
            $allLocalUpToDate = $false
            break
        }
        $trackedVersion = $null
        if ($fileVersions.ContainsKey($file)) {
            $trackedVersion = [string]$fileVersions[$file]
        }
        if ($trackedVersion -ne $latestVersion) {
            $allLocalUpToDate = $false
            break
        }
    }

    $syncScope = if ([string]::IsNullOrWhiteSpace($Board)) { "all" } else { $Board }
    if (-not $Force -and $allLocalUpToDate) {
        Write-Info "Already up to date. scope=$syncScope version=$latestVersion"
        exit 0
    }

    Write-Info "Sync start. scope=$syncScope local=$localVersion remote=$latestVersion"

    $tempDir = Join-Path $env:TEMP ("vendor_sdk_sync_" + [Guid]::NewGuid().ToString("N"))
    Ensure-Dir -Path $tempDir

    try {
        $manifestByName = @{}
        try {
            Write-Info "Download manifest: $manifestFile"
            $manifestPath = Join-Path $tempDir $manifestFile
            $manifestUrl = Get-PackageFileUrl -ApiV4Url $apiV4Url -ProjectIdOrPath $projectId -PackageName $packageName -PackageVersion $latestVersion -FileName $manifestFile
            Download-FileWithRetry -Uri $manifestUrl -Headers $headers -OutFile $manifestPath -Label $manifestFile -ShowProgress -TimeoutSeconds $NetworkTimeoutSeconds
            $manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
            if ($null -ne $manifest.files) {
                foreach ($entry in $manifest.files) {
                    if ($entry.name -and $entry.sha256) {
                        $manifestByName[[string]$entry.name] = [string]$entry.sha256
                    }
                }
            }
        } catch {
            Write-Info "Manifest not found or invalid, continue without checksum manifest."
        }

        $downloadCount = @($expectedFiles).Count
        $downloadIndex = 0
        foreach ($file in @($expectedFiles)) {
            $downloadIndex++
            Write-StepProgress -Stage "Download library" -Index $downloadIndex -Total $downloadCount -Item $file
            $targetTemp = Join-Path $tempDir $file
            $downloadUrl = Get-PackageFileUrl -ApiV4Url $apiV4Url -ProjectIdOrPath $projectId -PackageName $packageName -PackageVersion $latestVersion -FileName $file
            Download-FileWithRetry -Uri $downloadUrl -Headers $headers -OutFile $targetTemp -Label $file -ShowProgress -TimeoutSeconds $NetworkTimeoutSeconds

            if ($manifestByName.ContainsKey($file)) {
                $actual = (Get-FileHash -LiteralPath $targetTemp -Algorithm SHA256).Hash.ToLowerInvariant()
                $expect = $manifestByName[$file].ToLowerInvariant()
                if ($actual -ne $expect) {
                    throw "Checksum mismatch for $file"
                }
            }
        }

        $installCount = @($expectedFiles).Count
        $installIndex = 0
        foreach ($file in @($expectedFiles)) {
            $installIndex++
            Write-StepProgress -Stage "Install library" -Index $installIndex -Total $installCount -Item $file
            Move-Item -LiteralPath (Join-Path $tempDir $file) -Destination (Join-Path $libDir $file) -Force
            $fileVersions[$file] = [string]$latestVersion
        }

        $stateFileVersions = [ordered]@{}
        $stateFiles = @($fileVersions.Keys | Sort-Object)
        foreach ($stateFile in @($stateFiles)) {
            $stateFileVersions[[string]$stateFile] = [string]$fileVersions[[string]$stateFile]
        }
        $newState = [ordered]@{
            version   = $latestVersion
            package   = $packageName
            synced_at = (Get-Date).ToString("yyyy-MM-ddTHH:mm:ssK")
            files     = $stateFiles
            file_versions = $stateFileVersions
        }
        ($newState | ConvertTo-Json -Depth 6) | Set-Content -LiteralPath $statePath -Encoding UTF8
    } catch {
        if (-not $Force -and $allLocalExists) {
            $reason = Get-ErrorMessage -Err $_
            Write-Info "SDK download failed, keep local SDK libraries. scope=$syncScope local=$localVersion reason=$reason"
            exit 0
        }
        throw
    } finally {
        if (Test-Path -LiteralPath $tempDir) {
            Remove-Item -LiteralPath $tempDir -Recurse -Force
        }
    }

    Write-Info "Sync complete. scope=$syncScope version=$latestVersion"
    exit 0
}
catch {
    $msg = Get-ErrorMessage -Err $_
    Write-Error "[sdk-sync] $msg"
    exit 1
}
