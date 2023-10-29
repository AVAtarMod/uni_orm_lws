if ([Environment]::GetEnvironmentVariable('CMAKE_DEFAULT_INCLUDE_PATH'))
{
   git clone "https://github.com/AVAtarMod/lib_simplex_io" $Env:CMAKE_DEFAULT_INCLUDE_PATH
   if ($? != 0)
   {
      Write-Output "please run command `ngit clone https://github.com/AVAtarMod/lib_simplex_io $$CMAKE_DEFAULT_INCLUDE_PATH`n in git bash"
   }
} else {
   Write-Output "environment variable CMAKE_DEFAULT_INCLUDE_PATH not exist!"
   exit 1
}
