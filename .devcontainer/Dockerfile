# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.187.0/containers/cpp/.devcontainer/base.Dockerfile

# [Choice] Debian / Ubuntu version: debian-10, debian-9, ubuntu-20.04, ubuntu-18.04
ARG VARIANT="debian-11"
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}

# [Optional] Uncomment this section to install additional packages.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
#    && apt-get install -y mongodb-org-server mongodb-org-shell \
    && apt-get -y install --no-install-recommends icu-devtools \
               clang-format \
               clang-tidy \
#               libboost-all-dev \
               libssl-dev \
               openssl \
               cmake-curses-gui \
               python3-pip \
    && apt-get clean -y && rm -rf /var/lib/apt/lists/*; 

RUN pip install conan