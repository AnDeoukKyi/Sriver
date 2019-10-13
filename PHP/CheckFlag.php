<?php
 	header("Connect-Type: text/html; charset=UTF-8");
	$con = mysqli_connect("localhost","nejoo97","njeunjoo1997!","nejoo97");

	$ID = $_GET["ID"];

	$result = mysqli_query($con, "SELECT FLAG FROM MEMBER WHERE ID = '$ID'");


	$response = array();
    	while($row = mysqli_fetch_array($result)){
        	array_push($response, array("FLAG"=>$row[0]));
    	}

 	echo json_encode(array("response"=>$response), JSON_UNESCAPED_UNICODE);
    mysqli_close($con);
?>

