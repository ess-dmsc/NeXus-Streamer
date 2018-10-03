@Library('ecdc-pipeline')
import ecdcpipeline.ContainerBuildNode
import ecdcpipeline.PipelineBuilder

project = "NeXus-Streamer"

// Set number of old artefacts to keep.
properties([
    buildDiscarder(
        logRotator(
            artifactDaysToKeepStr: '',
            artifactNumToKeepStr: '5',
            daysToKeepStr: '',
            numToKeepStr: ''
        )
    )
])

clangformat_os = "debian9"
test_and_coverage_os = "centos7"
archive_os = "centos7"
release_os = "centos7-release"

container_build_nodes = [
    'centos7': new ContainerBuildNode('essdmscdm/centos7-build-node:3.2.0', '/usr/bin/scl enable rh-python35 devtoolset-6 -- /bin/bash -e'),
    'centos7-release': new ContainerBuildNode('essdmscdm/centos7-build-node:3.2.0', '/usr/bin/scl enable rh-python35 devtoolset-6 -- /bin/bash -e'),
    'debian9': new ContainerBuildNode('essdmscdm/debian9-build-node:2.3.0', 'bash -e'),
    'ubuntu1804': new ContainerBuildNode('essdmscdm/ubuntu18.04-build-node:1.2.0', 'bash -e')
]

pipeline_builder = new PipelineBuilder(this, container_build_nodes)
pipeline_builder.activateEmailFailureNotifications()
pipeline_builder.activateSlackFailureNotifications()

builders = pipeline_builder.createBuilders { container ->

    pipeline_builder.stage("${container.key}: checkout") {
        dir(pipeline_builder.project) {
            scm_vars = checkout scm
        }
        // Copy source code to container
        container.copyTo(pipeline_builder.project, pipeline_builder.project)
    }  // stage

    pipeline_builder.stage("${container.key}: get dependencies") {
        container.sh """
            mkdir build
            cd build
            conan remote add --insert 0 ess-dmsc-local ${local_conan_server}
        """
    }  // stage

    pipeline_builder.stage("${container.key}: configure") {
        if (container.key != release_os) {
            def coverage_on
            if (container.key == test_and_coverage_os) {
                coverage_on = "-DCOV=1"
            } else {
                coverage_on = ""
            }

            def cmake_cmd
            if (container.key == "centos7") {
                cmake_cmd = "cmake3"
            } else {
                cmake_cmd = "cmake"
            }

            container.sh """
                cd build
                ${cmake_cmd} ../${pipeline_builder.project} ${coverage_on}
            """
        } else {
            container.sh """
                cd build
                cmake3 -DCMAKE_SKIP_BUILD_RPATH=ON -DCMAKE_BUILD_TYPE=Release ../${pipeline_builder.project}
            """
        }  // if/else
    }  // stage

    pipeline_builder.stage("${container.key}: build") {
        container.sh """
            cd build
            . ./activate_run.sh
            make all UnitTests VERBOSE=1
        """
    }  // stage

    pipeline_builder.stage("${container.key}: test") {
        if (container.key == test_and_coverage_os) {
            // Run tests with coverage.
            def test_output = "TestResults.xml"
            container.sh """
                cd build
                . ./activate_run.sh
                ./bin/UnitTests ../${pipeline_builder.project}/data/ --gtest_output=xml:${test_output}
                make coverage
                lcov --directory . --capture --output-file coverage.info
                lcov --remove coverage.info '*_generated.h' '*/.conan/data/*' '*/usr/*' '*Test.cpp' '*gmock*' '*gtest*' --output-file coverage.info
            """

            container.copyFrom('build', '.')
            junit "build/${test_output}"

            withCredentials([string(credentialsId: 'nexus-streamer-codecov-token', variable: 'TOKEN')]) {
                sh "cp ${pipeline_builder.project}/codecov.yml codecov.yml"
                sh "curl -s https://codecov.io/bash | bash -s - -f build/coverage.info -t ${TOKEN} -C ${scm_vars.GIT_COMMIT}"
            }  // withCredentials
        } else {
            // Run tests.
            container.sh """
                cd build
                . ./activate_run.sh
                ./bin/UnitTests ../${pipeline_builder.project}/data/
            """
        }  // if/else
    }  // stage

    if (container.key != release_os) {
        pipeline_builder.stage("${container.key}: archive") {
            container.sh """
                mkdir -p archive/${pipeline_builder.project}
                cp -r build/bin archive/${pipeline_builder.project}
                cp -r build/lib archive/${pipeline_builder.project}
                cp -r build/licenses archive/${pipeline_builder.project}
                cp -r ${pipeline_builder.project}/data archive/${pipeline_builder.project}
                cd archive
                tar czvf ${pipeline_builder.project}-${container.key}.tar.gz ${pipeline_builder.project}
            """
            container.copyFrom("${pipeline_builder.project}-${container.key}.tar.gz", '.')
            archiveArtifacts "${pipeline_builder.project}-${container.key}.tar.gz"
        }  // stage
    }  // if

    if (container.key == clangformat_os) {
        container.sh """
            clang-format -version
            cd ${pipeline_builder.project}
            find . \\\\( -name '*.cpp' -or -name '*.cxx' -or -name '*.h' -or -name '*.hpp' \\\\) \\
                -exec clangformatdiff.sh {} +
        """
    }  // if

}  // createBuilders

node('docker') {
    // Delete workspace when build is done.
    cleanWs()

    stage('Checkout') {
        dir("${project}") {
            try {
                scm_vars = checkout scm
            } catch (e) {
                failure_function(e, 'Checkout failed')
            }
        }
    }

    builders['macOS'] = get_macos_pipeline()

    try {
        parallel builders
    } catch (e) {
        pipeline_builder.handleFailureMessages()
        throw e
    }
}

def failure_function(exception_obj, failureMessage) {
    def toEmails = [[$class: 'DevelopersRecipientProvider']]
    emailext body: '${DEFAULT_CONTENT}\n\"' + failureMessage + '\"\n\nCheck console output at $BUILD_URL to view the results.', recipientProviders: toEmails, subject: '${DEFAULT_SUBJECT}'
    slackSend color: 'danger', message: "${project}: " + failureMessage
    throw exception_obj
}

def get_macos_pipeline()
{
    return {
        stage("macOS") {
            node ("macos") {
                // Delete workspace when build is done
                cleanWs()

                dir("${project}/code") {
                    try {
                        checkout scm
                    } catch (e) {
                        failure_function(e, 'MacOSX / Checkout failed')
                    }
                }

                dir("${project}/build") {
                    try {
                        sh "cmake ../code"
                    } catch (e) {
                        failure_function(e, 'MacOSX / CMake failed')
                    }

                    try {
                        sh "make all UnitTests VERBOSE=1"
                        sh ". ./activate_run.sh && ./bin/UnitTests ../code/data/ --gtest_output=xml:TestResults.xml"
                    } catch (e) {
                        failure_function(e, 'MacOSX / build+test failed')
                    }
                }
            }
        }
    }
}
