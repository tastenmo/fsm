pipeline {
agent {
    // Equivalent to "docker build -f Dockerfile.build --build-arg version=1.0.2 ./build/
    dockerfile {
        dir '.devcontainer'
        
    }
}
	stages {
        stage('Build') {
            steps {
                sh 'cmake -S . -B build'
                sh 'cmake --build build'
            }
        }

        stage('Test') {
            steps {
                //sh 'cmake --build . --target test-cpp17'
                sh 'ctest --test-dir build'
            }
        }

}
}