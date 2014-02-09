# Pulls in and compiles the bandit source from it's git repository
include(ExternalProject)

ExternalProject_Add(
    bandit
    PREFIX dependencies
    GIT_REPOSITORY https://github.com/joakimkarlsson/bandit.git
    #GIT_REPOSITORY /home/matiu/projects/bandit
    INSTALL_COMMAND echo # No install
)
