from setuptools import setup, find_packages

setup(
    name='remote_config',
    version='0.1.0',
    description='Remote config client and server',
    long_description="",
    author='Oleksandr Slovak',
    author_email='slovak194@gmail.com',
    url="https://github.com/slovak194/remote-config",
    license="MIT License",
    packages=find_packages(exclude=('tests', 'docs')),
    package_data={},
    scripts=[]
)
