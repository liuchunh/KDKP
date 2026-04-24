需要陀螺仪与gps结合 

从 `GnssData` 中获取与下一个点的距离 `GnssData.DistanceToNextNode` 和角度 `GnssData.DegreeToNextNode`

从 `struct ImuData` 中获取当前的角速度 `ImuData.GyroX, GyroY, GyroZ` 和 加速度 `AccX, AccY, AccZ`

对这两个值进行积分 得到当前的速度 和 位置

通过显示屏显示当前的速度 和 位置

至于速度可以使用 `PID`
