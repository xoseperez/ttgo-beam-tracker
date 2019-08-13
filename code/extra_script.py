from shutil import copyfile

Import("env", "projenv")

# Get current environment
environment = env['PIOENV']
libdeps = env['PROJECTLIBDEPS_DIR']
file = "lmic_project_config.h"
source = "src/{}".format(file)
destination = "{}/{}/MCCI LoRaWAN LMIC library/project_config/{}".format(libdeps, environment, file)
copyfile(source, destination)
