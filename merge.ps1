Get-Content picture_array.h, helper_funcs.h, text_editor.h, visual_system.h, picture_array.c, helper_funcs.c, text_editor_v2.c, visual_system.c, shell_v7.c |
    Where-Object { $_ -notmatch '#include "' } |
    Set-Content merged.c

Write-Host "Done! Output: merged.c"

# navigate to directory where this shell and all files are in
# .\merge.ps1 to run the script