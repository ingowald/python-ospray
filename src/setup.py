from distutils.core import setup, Extension

ospraymodule = Extension('ospray',
                    define_macros = [('MAJOR_VERSION', '1'),
                                     ('MINOR_VERSION', '0')],
                    include_dirs = ['/usr/local/include'],
                    libraries = ['ospray'],
                    library_dirs = ['/usr/local/lib'],
                    sources = ['PythonBindings.cpp'])

setup (name = 'ospray',
       version = '1.0',
       description = 'This is a demo package',
       author = 'Ingo Wald',
       author_email = 'ingowald@gmail.com',
       url = 'https://docs.python.org/extending/building',
       long_description = '''
This is really just a demo package.
''',
       ext_modules = [ospraymodule])
