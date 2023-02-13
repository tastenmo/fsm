
ARG VARIANT="bullseye-slim"
FROM debian:${VARIANT}

ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Create the user
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    #
    # [Optional] Add sudo support. Omit if you don't need to install software after connecting.
    && apt-get update \
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME



# Install needed packages. Use a separate RUN statement to add your own dependencies.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install build-essential curl wget gpg git cmake ninja-build  \
    && apt-get autoremove -y && apt-get clean -y && rm -rf /var/lib/apt/lists/*


# Install llvm default packages
#RUN curl "https://apt.llvm.org/llvm-snapshot.gpg.key" | apt-key add - \
RUN curl "https://apt.llvm.org/llvm-snapshot.gpg.key" | gpg --dearmor | tee /usr/share/keyrings/llvm-snapshot.gpg > /dev/null 2>&1 \
    && echo "deb [signed-by=/usr/share/keyrings/llvm-snapshot.gpg] http://apt.llvm.org/bullseye/ llvm-toolchain-bullseye-16 main" | tee /etc/apt/sources.list.d/clang16.list \
    && echo "deb-src [signed-by=/usr/share/keyrings/llvm-snapshot.gpg] http://apt.llvm.org/bullseye/ llvm-toolchain-bullseye-16 main" | tee -a /etc/apt/sources.list.d/clang16.list \
    && apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install clang-format-16 \
                    clang-tidy-16 \
                    clang-tools-16 \
                    clang-16 \
                    clangd-16 \
                    libc++-16-dev \
                    libc++1-16 \
                    libc++abi-16-dev \
                    libc++abi1-16 \
                    libclang-16-dev \
                    libclang1-16 \
                    liblldb-16-dev \
                    libllvm-16-ocaml-dev \
                    libomp-16-dev \
                    libomp5-16\
                    lld-16\
                    lldb-16 \
                    llvm-16-dev \
                    llvm-16-runtime \
                    llvm-16 \
                    python3-clang-16

#USER $USERNAME