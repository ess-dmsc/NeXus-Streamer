@Library('ecdc-pipeline')
import ecdcpipeline.ContainerBuildNode
import ecdcpipeline.PipelineBuilder

project = "NeXus-Streamer"

// Set number of old artifacts to keep.
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

clangformat_os = "debian11"
test_and_coverage_os = "centos7"
archive_os = "centos7"
release_os = "centos7-release"

container_build_nodes = [
  'centos7': ContainerBuildNode.getDefaultContainerBuildNode('centos7-gcc11'),
  'centos7-release': ContainerBuildNode.getDefaultContainerBuildNode('centos7-gcc11'),
  'debian11': ContainerBuildNode.getDefaultContainerBuildNode('debian11'),
  'ubuntu2204': ContainerBuildNode.getDefaultContainerBuildNode('ubuntu2204')
]

pipeline_builder = new PipelineBuilder(this, container_build_nodes)
pipeline_builder.activateEmailFailureNotifications()

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
                coverage_on = ""
            } else {
                coverage_on = ""
            }

            container.sh """
                cd build
                cmake -DCMAKE_BUILD_TYPE=Debug ../${pipeline_builder.project} ${coverage_on}
            """
        } else {
            container.sh """
                cd build
                cmake -DCMAKE_SKIP_BUILD_RPATH=ON -DCMAKE_BUILD_TYPE=Release ../${pipeline_builder.project}
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
                ./bin/UnitTests -d ../${pipeline_builder.project}/data/ --gtest_output=xml:${test_output}
            """

            container.copyFrom('build', '.')
            junit "build/${test_output}"
        } else {
            // Run tests.
            container.sh """
                cd build
                . ./activate_run.sh
                ./bin/UnitTests -d ../${pipeline_builder.project}/data/
            """
        }  // if/else
    }  // stage

    if (container.key == release_os) {
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
            container.copyFrom("archive/${pipeline_builder.project}-${container.key}.tar.gz", '.')
            archiveArtifacts "${pipeline_builder.project}-${container.key}.tar.gz"
        }  // stage
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
                        // Conan remove is temporary until all projects have moved to lowercase package name
                        sh "conan remove -f FlatBuffers/*"
                        checkout scm
                    } catch (e) {
                        failure_function(e, 'MacOSX / Checkout failed')
                    }
                }

                dir("${project}/build") {
                    try {
                        sh "cmake -DCMAKE_BUILD_TYPE=Debug ../code"
                    } catch (e) {
                        failure_function(e, 'MacOSX / CMake failed')
                    }

                    try {
                        sh "make all UnitTests VERBOSE=1"
                        sh ". ./activate_run.sh && ./bin/UnitTests -d ../code/data/ --gtest_output=xml:TestResults.xml"
                    } catch (e) {
                        failure_function(e, 'MacOSX / build+test failed')
                    }
                }
            }
        }
    }
}
