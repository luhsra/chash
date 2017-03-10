<?php

final class TAPTestEngine extends ArcanistUnitTestEngine {

  public function run() {
    $projectRoot = $this->getWorkingCopy()->getProjectRoot();
    $command = $this->getConfigurationManager()->getConfigFromAnySource('unit.engine.tap.command');

    $future = new ExecFuture($command);
    $future->setCWD(Filesystem::resolvePath($projectRoot));

    do {
      list($stdout, $stderr) = $future->read();
      echo $stdout;
      echo $stderr;
      sleep(0.5);
    } while (!$future->isReady());

    list($error, $stdout, $stderr) = $future->resolve();
    return $this->parseOutput($stdout);
  }

  public function shouldEchoTestResults() {
    return true;
  }

  private function parseOutput($output) {
    $results = array();
    $lines = explode(PHP_EOL, $output);

    $result = null;
    $userData = "";
    
    foreach($lines as $index => $line) {
      preg_match('/^(not ok|ok)\s+\d+\s+-?(.*)/', $line, $matches);
      if (count($matches) < 3) {
         preg_match('/^#\s*duration\s([0-9.]+)/', $line, $matches);
         if ($result != null && count($matches) == 2) {
            $result->setDuration((float)$matches[1]);
         }
         preg_match('/^#(.*)/', $line, $matches);
         if ($result != null && count($matches) == 2) {
            $userData .= $matches[1] . "\n";
            $result->setUserData($userData);
         }
         continue;
      } 

      $result = new ArcanistUnitTestResult();
      $result->setName(trim($matches[2]));
      $userData = "";

      switch (trim($matches[1])) {
        case 'ok':
          $result->setResult(ArcanistUnitTestResult::RESULT_PASS);
          break;

        case 'not ok':
          $exception_message = trim($lines[$index + 1]);
          $result->setResult(ArcanistUnitTestResult::RESULT_FAIL);
          break;

        default:
          continue;
      }

      $results[] = $result;
    }

    return $results;
  }
}
