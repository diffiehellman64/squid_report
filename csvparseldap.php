<?php
function get_users($org) {
	global $pass;
        $ldapserver = 'syk-kom8-dc10.rk.local';
        $ldapuser = 'yav004@RK.LOCAL';
        $ldappass = $pass;
        $ldaptree = "OU=Users,OU=".$org.",OU=Resources,DC=rk,DC=local";

        $ldapconn = ldap_connect($ldapserver) or die("Could not connect to LDAP server.");

        if($ldapconn) {
                $ldapbind = ldap_bind($ldapconn, $ldapuser, $ldappass) or die ("Error trying to bind: ".ldap_error($ldapconn));
                if ($ldapbind) {
                        $result = ldap_search($ldapconn, $ldaptree, "(cn=*)") or die ("Error in search query: ".ldap_error($ldapconn));
                        $data = ldap_get_entries($ldapconn, $result);
                        $result_arr = array();
                                for ($i=0; $i<$data["count"]; $i++) {
                                        if (isset($data[$i]['samaccountname'][0])){
                                                $result_arr[$data[$i]['samaccountname'][0]] = $data[$i]['cn'][0];
                                        }
                                }

                } else {
                         echo "LDAP bind failed...";
                }
        }
        ldap_close($ldapconn);
        return $result_arr;
}

function get_user_fio($user){
	global $pass;
        $ldapserver = 'syk-kom8-dc10.rk.local';
        $ldapuser = 'yav004@RK.LOCAL';
        $ldappass = $pass;
        $ldaptree = "OU=Resources,DC=rk,DC=local";

        $ldapconn = ldap_connect($ldapserver) or die("Could not connect to LDAP server.");

        if($ldapconn) {
                $ldapbind = ldap_bind($ldapconn, $ldapuser, $ldappass) or die ("Error trying to bind: ".ldap_error($ldapconn));
                if ($ldapbind) {
                        $result = ldap_search($ldapconn, $ldaptree, "(sAMAccountName=$user)") or die ("Error in search query: ".ldap_error($ldapconn));
                        $data = ldap_get_entries($ldapconn, $result);
			if (isset($data[0]['cn'][0])){
				$user_fio = $data[0]['cn'][0];
			} else {
				$user_fio = 'unnamed';
			}
                } else {
                         echo "LDAP bind failed...\n";
                }
        }
        ldap_close($ldapconn);
        return $user_fio;
}

function get_org($user){
	global $pass;
        $ldapserver = 'syk-kom8-dc10.rk.local';
        $ldapuser = 'yav004@RK.LOCAL';
        $ldappass = $pass;
        $ldaptree = "OU=Resources,DC=rk,DC=local";

        $ldapconn = ldap_connect($ldapserver) or die("Could not connect to LDAP server.");

        if($ldapconn) {
                $ldapbind = ldap_bind($ldapconn, $ldapuser, $ldappass) or die ("Error trying to bind: ".ldap_error($ldapconn));
                if ($ldapbind) {
                        $result = ldap_search($ldapconn, $ldaptree, "(sAMAccountName=$user)") or die ("Error in search query: ".ldap_error($ldapconn));
                        $data = ldap_get_entries($ldapconn, $result);
			if (isset($data[0]['dn'])) {
				$dn = split(',', $data[0]['dn']);
				$org = substr($dn[2], 3);
			} else {
				$org = 'udentified';
			}
//			echo $org;
                } else {
                         echo "LDAP bind failed...\n";
                }
        }
        ldap_close($ldapconn);
        return $org;
}

function get_org_from_cache($user, $cache_file){
	$org = exec ("grep $user $cache_file | awk -F '|' '{print $2}'");
	return $org;
}

function add_cache($cache_file, $str){
	$fp = fopen($cache_file, 'a');
//	echo "adding to cache ". $str ."\n";
        fputs ($fp, $str);
        fclose ($fp);
}

function get_csv ($arr, $csvfilename){
	echo "\nCreating CSV file...\n";
	global $monitor;
	global $users;	
	global $time_str;	
	$line = 'fio;AccountName;';
	for ($i=0; $i < count($monitor); $i++ ){
		$line .= $monitor[$i].';';
	}
	$fp = fopen($csvfilename, "w");
	fputs ($fp, "Log period: ".date("d/m/Y", $time_str[0]) ." - ". date("d/m/Y", $time_str[1]) ."\n");
	fputs ($fp, $line ."\n");
	foreach ($arr as $user => $visits){
        	$line = $users[$user].';'.$user.';';
	        foreach ($monitor as $site){
        	        $line .= isset($visits[$site])?$visits[$site]:0;
                	$line .= ';';
        	}
        	fputs ($fp, $line."\n");
	}
	fclose ($fp);
}

function get_global_csv ($arr, $csvfilename){
	echo "\nCreating global CSV file...\n";
//	print_r($arr);
        global $monitor;
        $line = 'fio;AccountName;';
        for ($i=0; $i < count($monitor); $i++ ){
                $line .= $monitor[$i].';';
        }
        $fp = fopen($csvfilename, "w");
        fputs ($fp, $line."\n");
	foreach ($arr as $org => $users){
		fputs ($fp, $org."\n");
	        foreach ($users as $user => $visits){
        	        $line = get_user_fio($user).';'.$user.';';
	                foreach ($monitor as $site){
                       		$line .= isset($visits[$site])?$visits[$site]:0;
                        	$line .= ';';
                	}
                	fputs ($fp, $line."\n");
        	}
	}
        fclose ($fp);
}


function get_sites($listfile){
	$result = array();
	$handle = @fopen($listfile, "r");
        if ($handle) {
		while (($buffer = fgets($handle, 4096)) !== false) {
			$result[] = trim($buffer);
		}
	}
	if (!feof($handle)) {
        	echo "Error: unexpected fgets() fail\n";
	}
	fclose($handle);
	$result[] = 'other';
	return $result;
}

function parsecsv($csvfile){
	$handle = @fopen($csvfile, "r");
        if ($handle) {
		while (($buffer = fgets($handle, 4096)) !== false) {
			$user = substr($buffer, 0, stripos($buffer, ";"));
			if ($user == 'user') {
				$line = 'org;fio;'.$buffer;
			} else {
				$line = get_org($user).";".get_user_fio($user).";".$buffer;
			}
			echo $line;
		}
	}
	if (!feof($handle)) {
        	echo "Error: cann`t open csv file\n";
	}
	fclose($handle);
}

function get_clen_baseurl($long_url){
	preg_match('/^(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}).*/', $long_url, $match);
	if (isset($match[1])){
		$url = $match[1];
	} else {
		$url_arr = parse_url($long_url);
		$pos = strrpos($url_arr['host'], '.');
		$url = substr($url_arr['host'], $pos);
		$url2 = substr($url_arr['host'], 0, $pos);
        	$pos = strrpos($url2, '.');
	        if ($pos === false){
        		$url = $url2.$url;
		} else {
			$url = substr($url2, $pos+1).$url;
		}
	}
	return $url;
}

function show_progress($count, $current_line){
        global $show_prg;
        $prg = $current_line/$count * 100;
        if ($prg - $show_prg > 0.01){
                $show_prg = $prg;
                $l = 50;
                $bar = $prg/(100/50);
                echo "[";
                for ($i=0; $i<$l; $i++){
                        if ($i < $bar){
                                echo '#';
                        } else {
                                echo ".";
                        }
                }
                echo "]  ". round($prg, 2). "%\r";
        }
}

function get_time_pool($time){
	global $time_str;
	if ($time_str[0] > $time) {$time_str[0] = $time; echo '0 = '.$time."\n";}
	if ($time_str[1] < $time) {$time_str[1] = $time; echo '1 = '.$time."\n"; }
}

function get_result($logfile){
	global $users;
	global $monitor;
        global $show_prg;
	global $time_str;
	$time_str[0] = 9999999999;
	$time_str[1] = 0;
	$current_line = 0;
        $show_prg = 0;
        $count = exec("wc -l $logfile");
        $count = split(' ', $count);
	$result = array();
	$urls = array();
	$handle = @fopen("$logfile", "r");
	if ($handle) {
		while (($buffer = fgets($handle, 4096)) !== false) {
			$current_line++;
                        show_progress($count[0], $current_line);
			$line = split(" \| ", trim($buffer));
			$response = substr($line[3], 0, 8);
			if ($line[5] == 'GET' && $line[9] == 'text/html' && $response == 'TCP_MISS'){
				get_time_pool($line[0]);
//				echo strrpos($line[7], '@')."\n";
				if (strrpos($line[7], '@')){
					$user = substr($line[7], 0, strrpos($line[7], '@'));
				} else {
					$user = $line[7];
				}
//				echo $user."\n";
//				$user = substr($line[7], 0, strrpos($line[7], '@'));
				if (isset($users[$user])){
					$url = get_clen_baseurl($line[6]);
					if (!in_array($url, $monitor)){
						$url = 'other';
					}

					if (!isset ($result[$user])){
						$result[$user] = array();
					}
        
					if (!isset ($result[$user][$url])){
        			        	$result[$user][$url] = 0;
			        	}
					$result[$user][$url]++;
				} else {
	//				echo $buffer;
				}
			}
    		}
		if (!feof($handle)) {
        		echo "Error: unexpected fgets() fail\n";
		}
		fclose($handle);
	}
	return $result;
}

function get_global_result($logfile){
        global $monitor;
	global $show_prg;
	$show_prg = 0;
	$count = exec("wc -l $logfile");
	$count = split(' ', $count);
        $result = array();
        $urls = array();
	$current_line = 1;
        $handle = @fopen($logfile, "r");
        if ($handle) {
                while (($buffer = fgets($handle, 4096)) !== false) {
			$current_line++;			
			show_progress($count[0], $current_line);
                        $tmp = split('"', $buffer);
                        $l = trim($tmp[0]);
                        $line = preg_split('/ +/', $l);
                        preg_match('/^([^@]+)/', $line[7], $matches);
                        $user = $matches[1];
                        $url = get_clen_baseurl($line[6]);
			if ($user == '-' || $user == 'none'){
				$org = 'NONE';
			} else {
				$org = get_org_from_cache($user, 'users.cache');
				if (!$org){
					$org = get_org($user);
					add_cache('users.cache', "$user|$org\n");
				}
			}
                        if (!in_array($url, $monitor)){
                        	$url = 'other';
                        }
			
			if (!isset($result[$org])){
				$result[$org] = array();
			}

                        if (!isset ($result[$org][$user])){
                        	$result[$org][$user] = array();
                        }

                        if (!isset ($result[$org][$user][$url])){
                        	$result[$org][$user][$url] = 0;
                        }

                        $result[$org][$user][$url]++;
                } 
                if (!feof($handle)) {
                        echo "Error: unexpected fgets() fail\n";
                }
                fclose($handle);
        }
        return $result;
}
/*
$input = $argv[1];
$output = $argv[2];
$org = $argv[3];
$pass = $argv[4];

if (isset($input) && isset($output) && isset($org)){
	$monitor = get_sites('monitor_sites.list');
	echo "Start script...\nInput log file: ".$input."\nOutput csv file: ".$output."\nOrganization: ".$org."\n";
	echo "Parsing log file...\n";
	if ($org == 'GLOBAL'){
		get_global_csv(get_global_result($input), $output);
	} else {
		$users = get_users($org);
		get_csv(get_result($input), $output);
	}
	echo "DONE!\n";
} else {
	echo "You must use format: inputfile outputfile organization\n";
}
*/
$csvfile = $argv[1];
$pass = $argv[2];
parsecsv($csvfile);
