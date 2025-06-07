<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta http-equiv="refresh" content="30">
    <style type="text/css">
        .spec {
            text-align: center;
        }
        .con {
            text-align: left;
        }
    </style>
</head>

<body>
    <h1 align="center">IoT Room Status</h1>
    <div class="spec">
        # <b>Room status</b>
        <br><br>
    </div>

    <table border="1" style="width:50%" align="center">
        <tr align="center">
            <th>USER ID</th>
            <th>ROOM</th>
            <th>NAME</th>
            <th>STATUS</th>
        </tr>

        <?php
        $conn = mysqli_connect("localhost", "iot", "pwiot");
        if (!$conn) {
            die("Database connection failed: " . mysqli_connect_error());
        }

        mysqli_select_db($conn, "iotdb");
        $result = mysqli_query($conn, "SELECT user_id, room, name, status FROM room_status");

        while ($row = mysqli_fetch_array($result)) {
            echo "<tr align='center'>";
            echo "<td>" . htmlspecialchars($row['user_id']) . "</td>";
            echo "<td>" . htmlspecialchars($row['room']) . "</td>";
            echo "<td>" . htmlspecialchars($row['name']) . "</td>";
            echo "<td>" . htmlspecialchars($row['status']) . "</td>";
            echo "</tr>";
        }

        mysqli_close($conn);
        ?>
    </table>
</body>
</html>

