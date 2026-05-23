/**
 * @file sun_position.c
 * @brief 太阳位置计算函数（基于天文算法，精度约±0.5°）
 * @note  输入：经纬度（度）、UTC时间（年月日时分秒）
 *        输出：高度角、方位角（度）
 */

#include <math.h>

#ifndef PI
#define PI 3.141592653589793
#endif
#define RAD (PI/180.0)
#define DEG (180.0/PI)

/**
 * @brief 计算太阳高度角和方位角
 * @param lat     纬度（度，北正南负）
 * @param lon     经度（度，东正西负）
 * @param year    年（如2025）
 * @param month   月（1-12）
 * @param day     日（1-31）
 * @param hour    时（UTC，0-23）
 * @param minute  分（0-59）
 * @param second  秒（0-59）
 * @param alt     输出：太阳高度角（度）
 * @param az      输出：太阳方位角（度，正北顺时针0-360）
 */
 

void calculate_sun_position(double lat, double lon, int year, int month, int day,
                            int hour, int minute, int second,
                            double *alt, double *az)
{
    // 所有变量声明放在最前面
    long A, B;
    double JD, T, L0, M, e, C, L_true, nu, omega, lambda, eps;
    double alpha, delta, GMST, LST, H, sin_alt, cos_az, az_rad;

    // 1. 儒略日计算（可执行语句）
    if (month <= 2) {
        year--;
        month += 12;
    }
    A = year / 100;
    B = 2 - A + A / 4;
    JD = (long)(365.25 * (year + 4716)) + (int)(30.6001 * (month + 1)) + day + B - 1524.5;
    JD += (hour * 3600.0 + minute * 60.0 + second) / 86400.0;

    // 2. 儒略世纪数
    T = (JD - 2451545.0) / 36525.0;

    // 3. 太阳几何平黄经
    L0 = fmod(280.46646 + 36000.76983 * T + 0.0003032 * T * T, 360.0);

    // 4. 太阳平近点角
    M = fmod(357.52911 + 35999.05029 * T - 0.0001537 * T * T, 360.0);

    // 5. 地球离心率
    e = 0.016708634 - 0.000042037 * T - 0.0000001267 * T * T;

    // 6. 中心差
    C = (1.914602 - 0.004817 * T - 0.000014 * T * T) * sin(M * RAD)
      + (0.019993 - 0.000101 * T) * sin(2 * M * RAD)
      + 0.000289 * sin(3 * M * RAD);

    // 7. 太阳真黄经
    L_true = L0 + C;

    // 8. 真近点角
    nu = M + C;

    // 9. 视黄经
    omega = 125.04 - 1934.136 * T;
    lambda = L_true - 0.00569 - 0.00478 * sin(omega * RAD);

    // 10. 黄赤交角
    eps = 23.439291 - 0.013004 * T;

    // 11. 赤经、赤纬
    alpha = atan2(cos(eps * RAD) * sin(lambda * RAD), cos(lambda * RAD)) * DEG;
    delta = asin(sin(eps * RAD) * sin(lambda * RAD)) * DEG;

    // 12. 格林尼治恒星时
    GMST = fmod(280.46061837 + 360.98564736629 * (JD - 2451545.0)
              + 0.000387933 * T * T - T * T * T / 38710000.0, 360.0);

    // 13. 地方恒星时
    LST = GMST + lon;

    // 14. 时角
    H = LST - alpha;

    // 15. 高度角
    sin_alt = sin(lat * RAD) * sin(delta * RAD)
            + cos(lat * RAD) * cos(delta * RAD) * cos(H * RAD);
    *alt = asin(sin_alt) * DEG;

    // 16. 方位角
    cos_az = (sin(delta * RAD) - sin(lat * RAD) * sin_alt)
           / (cos(lat * RAD) * cos(*alt * RAD));
    if (cos_az > 1.0) cos_az = 1.0;
    if (cos_az < -1.0) cos_az = -1.0;
    az_rad = acos(cos_az);
    if (sin(H * RAD) > 0) {
        *az = 360.0 - az_rad * DEG;
    } else {
        *az = az_rad * DEG;
    }
}