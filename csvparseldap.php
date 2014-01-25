<?php
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

function parsecsv($csvfile, $out){
	$handle = @fopen($csvfile, "r");
        $fp = fopen($out, "w");
	if ($handle) {
                while (($buffer = fgets($handle, 4096)) !== false) {
                        $user = substr($buffer, 0, stripos($buffer, ";"));
                        if ($user == 'user') {
                                $line = 'org;fio;'.$buffer;
                        } else {
                                $line = get_org($user).";".get_user_fio($user).";".$buffer;
                        }
			echo $line;
                        fputs ($fp, $line);
                }
        }
	if (!feof($handle)) {
 		echo "Error: cann`t open csv file\n";
	}
	fclose($handle);
        fclose($fp);
}


function parseinput($out){
	$fp = fopen($out, "w");
	while ($buffer = fgets(STDIN)) {
		$user = substr($buffer, 0, stripos($buffer, ";"));
		if ($user == 'user') {
			$line = 'org;fio;'.$buffer;
		} else {
			$line = get_org($user).";".get_user_fio($user).";".$buffer;
		}
		fputs ($fp, $line);
	}
	fclose($fp);
}

$out = $argv[1];
$pass = $argv[2];
//parsecsv($csvfile, $out);
parseinput($out);
