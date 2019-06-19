<?php
    header("Connect-Type: text/html; charset=UTF-8");
    $con = mysqli_connect("localhost", "nejoo97", "njeunjoo1997!", "nejoo97");


    $POINT = $_GET["POINT"];
    $OCCUPY = $_GET["OCCUPY"];
    $STARTX = $_GET["STARTX"];
    $STARTY = $_GET["STARTY"];

    $result = mysqli_query($con, "INSERT INTO PARKINGPOINT VALUES ($POINT, $OCCUPY, $STARTX, $STARTY)");


    mysqli_stmt_bind_param($result, "iiii", $index, $occupy, $startX, $startY);
    mysqli_stmt_execute($result);



    mysqli_close($con);
?>



