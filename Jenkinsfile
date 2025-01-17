@Library('xmos_jenkins_shared_library@v0.18.0') _

getApproval()

pipeline {
  agent none

  environment {
    REPO = 'lib_spi'
    VIEW = getViewName(REPO)
  }

  options {
    skipDefaultCheckout()
  }

  stages {
    stage('Standard build and XS1/2 tests') {
      agent {
        label 'x86_64 && linux'
      }
      stages {
        stage('Get view') {
          steps {
            xcorePrepareSandbox("${VIEW}", "${REPO}")
          }
        }
        stage('Library checks') {
          steps {
            xcoreLibraryChecks("${REPO}")
          }
        }
        stage('Legacy tests') {
          steps {
            dir("${REPO}/legacy_tests") {
              viewEnv() {
                // Use Pipfile in legacy_tests, not lib_spi/Pipfile
                installPipfile(true)
                runPython("./runtests.py --junit-output=${REPO}_tests.xml")
              }
            }
          }
        }

        stage('Build XCOREAI') {
          steps {
            dir("${REPO}") {
              forAllMatch("examples", "AN*/") { path ->
                runXmake(path, '', 'XCOREAI=1')
                dir(path) {
                  stash name: path.split("/")[-1], includes: 'bin/*, '
                }
              }

              // Build Tests
              dir('legacy_tests/') {
                script {
                  tests = [
                    "spi_master_async_multi_client",
                    "spi_master_async_multi_device",
                    "spi_master_async_rx_tx",
                    "spi_master_async_shutdown",
                    "spi_master_sync_benchmark",
                    "spi_master_sync_clock_port_sharing",
                    "spi_master_sync_multi_client",
                    "spi_master_sync_multi_device",
                    "spi_master_sync_rx_tx",
                    "spi_slave_benchmark",
                    "spi_slave_rx_tx"
                  ]
                  tests.each() {
                    dir(it) {
                      //Build all tests tagetting XS3 to check for build errors only
                      runXmake(".", "", "XCOREAI=1")
                      stash name: it, includes: 'bin/**/*.xe, '
                    }
                  }
                }
              }
            }
          }
        }
      }//stages
      post {
        cleanup {
          xcoreCleanSandbox()
        }
      }
    }//stage - Standard build and XS1/2 tests

    stage('Documentation'){
      agent{
        label 'x86_64&&macOS'
      }
      stages{
        stage('Get view') {
          steps {
            xcorePrepareSandbox("${VIEW}", "${REPO}")
          }
        }
        stage('Builds') {
          steps {
            forAllMatch("${REPO}/examples", "AN*/") { path ->
              runXdoc("${path}/doc")
            }
            runXdoc("${REPO}/${REPO}/doc")

            // Archive all the generated .pdf docs
            archiveArtifacts artifacts: "${REPO}/**/pdf/*.pdf", fingerprint: true, allowEmptyArchive: true
          }
        }
      }
      post {
        cleanup {
          cleanWs()
        }
      }
    }

    stage('xcore.ai Verification'){
      agent {
        label 'xcore.ai-explorer'
      }
      stages{
        stage('Get view') {
          steps {
            xcorePrepareSandbox("${VIEW}", "${REPO}")
            dir("${REPO}") {
              viewEnv {
                withVenv {
                  sh "pip install -e ${WORKSPACE}/xtagctl"
                  sh "xtagctl reset_all XCORE-AI-EXPLORER"
                }
              }
            }
          }
        }
        stage('xrun'){
          steps{
            dir("${REPO}") {
              viewEnv {
                withVenv {
                  forAllMatch("examples", "AN*/") { path ->
                    unstash path.split("/")[-1]
                  }
                  // Run the tests and look for what we expect
                  sh 'xrun --io --id 0 bin/AN00160_using_SPI_master.xe &> AN00160_using_SPI_master.txt'
                  // Look for config register 0 value from wifi module
                  sh 'grep 2005400 AN00160_using_SPI_master.txt'

                  //Just run this and ensure we get no error (like wrong arch). We have no SPI master HW so cannot test it
                  sh 'xrun --id 0 bin/AN00161_using_SPI_slave.xe'
                }
              }
            }
          }
        }
      }//stages
      post {
        cleanup {
          cleanWs()
        }
      }
    }// xcore.ai

    stage('Update view files') {
      agent {
        label 'x86_64 && linux'
      }
      when {
        expression { return currentBuild.currentResult == "SUCCESS" }
      }
      steps {
        updateViewfiles()
      }
    }
  }//stages
}//pipeline
