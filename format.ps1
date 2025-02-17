function D14UIKit-Run-SortXML
{
    param($XMLFilePath)
    Write-Output "Format $XMLFilePath"
    & ".\\sortxml.exe" "--overwrite" $XMLFilePath | Out-Null
}
$executablePath = Get-Command "csc.exe" -ErrorAction SilentlyContinue
if ($executablePath)
{
    # Compile sortxml
    & $executablePath "sortxml.cs" | Out-Null

    # Format D14Engine
    # D14UIKit-Run-SortXML -XMLFilePath "D14Engine.vcxproj"
    # D14UIKit-Run-SortXML -XMLFilePath "D14Engine.vcxproj.filters"
    # D14UIKit-Run-SortXML -XMLFilePath "D14Engine.vcxproj.user"
}
else # Keep everything unchanged.
{
    Write-Output "Warning: csc.exe not found, no changes have been made."
}
