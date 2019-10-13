<?php
    header("Connect-Type: text/html; charset=UTF-8");
    $con = mysqli_connect("localhost", "nejoo97", "njeunjoo1997!", "nejoo97");


    $result = mysqli_query($con, "SELECT * FROM PARKINGPOINT");
    $response = array();
    while($row = mysqli_fetch_array($result)){
        array_push($response, array("POINT"=>$row[0], "OCCUPY"=>$row[1], "STARTX"=>$row[2], "STARTY"=>$row[3], "LENX"=>$row[4], "LENY"=>$row[5], "ID"=>$row[6]));
    }

    echo json_encode(array("response"=>$response), JSON_UNESCAPED_UNICODE);
    mysqli_close($con);
?>