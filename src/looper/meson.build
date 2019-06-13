if not get_option('unitTests')
  message('Create build config for publishing')
  loopp_src += files(
    'audiobuffer.cxx',
    'looperclip.cxx',
    'looper.cxx',
    'looperfactory.cxx'
    )

  if get_option('tests')
    message('Built with Diskwriter Tests')
    subdir('tests')
  endif
endif

if get_option('unitTests')
  message('Create build config for unit tests')
  subdir('unitTests')
endif