<?php
/* Setup DB connection */
define('DBNAME', '');
define('USERNAME', '');
define('PASSWORD', '');
$db = new PDO('mysql:host=localhost;dbname='.DBNAME, USERNAME, PASSWORD);
$db->query('SET NAMES UTF8');

$response = array();
switch ( strval( $_REQUEST['op'] ) ){
	case '':
	case '0':
		if ( !isset($_REQUEST['key']) )
            exit();
        /* 
         * for compatibility with r5, send phone-code instead of crc-code.
         * the crc-code mode is *DEPRECATED* now
         */

        /* select phrase by syllable */
        if ( is_int($_REQUEST['key']) ) {
    		$stmt = $db->prepare('SELECT id, phrase FROM corpus WHERE crc= :crc ORDER BY priority DESC');
    		$stmt->bindParam(':crc', $_REQUEST['key'], PDO::PARAM_STR);
        } else {
            $stmt = $db->prepare('SELECT id, phrase FROM corpus WHERE syllable= :syllable ORDER BY priority DESC');
            $stmt->bindParam(':syllable', $_REQUEST['key'], PDO::PARAM_STR);
        }
        $stmt->execute();

        /* build result and encode into json format */
		$response = array( $_REQUEST['key'] );
		foreach( $stmt->fetchAll() as $row ) {
			$result[ $row['id'] ] = $row['phrase'];
		}
		$response[] = $result;
        echo json_encode( $response );
		exit();
		break;
	default:
		break;
}
