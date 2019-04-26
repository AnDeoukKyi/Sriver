<?php
    header("Connect-Type: text/html; charset=UTF-8");
    $con = mysqli_connect("localhost", "nejoo97", "njeunjoo1997!", "nejoo97");


    $index = $_GET["index"];
    $occupy = $_GET["occupy"];
    $startX = $_GET["startX"];
    $startY = $_GET["startY"];
    $lenX = $_GET["lenX"];
    $lenY = $_GET["lenY"];

    $result = mysqli_query($con, "INSERT INTO PARKINGPOINT VALUES ($index, $occupy, $startX, $startY, $lenX, $lenY)");


    mysqli_stmt_bind_param($result, "iiiiii", $index, $occupy, $startX, $startY, $lenX, $lenY);
    mysqli_stmt_execute($result);


    $response = array();
    array_push($response, array("POINT"=>$result));

    echo json_encode(array("response"=>$response), JSON_UNESCAPED_UNICODE);

    mysqli_close($con);
?>



