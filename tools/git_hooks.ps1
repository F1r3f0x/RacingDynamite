param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('pre-commit', 'commit-msg')]
    [string]$Hook,
    [string]$MessagePath
)

$ErrorActionPreference = 'Stop'
if ($Hook -eq 'pre-commit') {
    & uv run --no-project --python 3.13 python tools/workflow.py check --staged
} else {
    if (-not $MessagePath) {
        throw 'commit-msg requires its Git message file'
    }
    & uv run --no-project --python 3.13 python tools/workflow.py commit-message $MessagePath
}
exit $LASTEXITCODE
