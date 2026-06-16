# 中文 OSD 行布局快速参考

## 概述

两个 Chinese OSD 数组分别对应屏幕同一行（ver_y=20）的左右两部分，每个数组 `unsigned short[32]`（索引 [0]~[31]）。

| 数组变量名 | 屏幕位置 | osd_idx | hor_x |
|-----------|---------|---------|-------|
| `osd_sensor_viewangle_enhance_viewstate_bricont_ir` | 左侧（行1） | 26 | 54 |
| `osd_inertia_velocitycompensation_workmode` | 右侧（行2） | 7 | 610 |

声明位置：`osd_graph_txt_chineseChar_app.h` 第113~114行  
初始化位置：`osd_graph_txt_chineseChar_app.cpp` 第54~55行  
屏幕配置位置：`osd_graph_txt_chineseChar_app.cpp` 第458~471行

---

## 汉字编码规则

每个汉字占 **2 个槽位**（`_1` 后缀为高字节，`_2` 后缀为低字节），ASCII 字符占 1 个槽位。

---

## 行1 当前槽位布局

```
索引        内容                    数据来源
[0]~[3]    传感器类型（2汉字）       channel_disp（0x02）
           电视=电视 / 红外=红外 / 光半=光半
[4]        空
[5]~[9]    放大倍率（5 ASCII）       magnification_str（0x26 Byte5~6）
           格式 "xx.xX"，不足右补空格
[10]       ' '（显式空格）
[11]~[15]  焦距值（5 ASCII）         focus_str（0x45）
           格式如 "12500"，不足补 0
[16]       空
[17]~[22]  视角值（6 ASCII）         yaw_view_angle_str（0x26）
[23]       空
[24]~[25]  增强等级（1汉字）         enhance_level（0x21 loc 0x03）
           '0'=空 / '1'=轻 / '2'=中 / '3'=重 / '4'=透
[26]       空
[27]~[30]  手动/自动（2汉字）        brigheness_contrast_modify（0x21 loc 0x05）
           '0'=手动 / '1'=自动
[31]       空
```

**写入触发条件**（main.cpp 约4774行）：
`level_changed || channel_disp变 || enhance_level变 || sensor_view_size变 || ir_power变 || laser_power变 || brigheness_contrast_modify变 || ir_pola变 || yaw_view_angle_str变 || magnification_str变`

**焦距值单独触发**（main.cpp 约5267行）：
`focus_str变` → 写 [11]~[15] → 单独 update 行1

---

## 行2 当前槽位布局

```
索引        内容                    数据来源
[0]~[3]    惯性状态（2汉字或空格）   inertial_state（0x21 loc 0x06）
           '0'或show_level==6=空 / '1'=有效
[4]~[7]    空
[8]~[10]   速度补偿（3 ASCII）       vel_comp（0x1b）
           1="LMC" / 2="   "
[11]~[13]  空
[14]~[21]  工作模式（最多4汉字）     work_mode（0x16）
           0x31=初始 / 0x32=手动 / 0x33=航测 / 0x34=回收
           0x35=自动扫描 / 0x36=锁定 / 0x37=图像跟踪
           0x38=跟搜 / 0x39=搜索 / 0x3a=锁定当前
           0x3b=地利跟踪 / 0x3c=随动 / 0x3d=前视
[22]~[25]  红外极性（2汉字）         ir_pola（0x19），channel==2且ir_power时有效
           0x05=黑热 / 0x06=白热 / 其他=空格
[26]~[31]  空
```

**写入触发条件**（main.cpp 约4974行）：
`level_changed || inertial_state变 || vel_comp变 || work_mode变`

**红外极性写入时机**：随行1的大触发块一起（main.cpp 约4921行），写 `osd_inertia_velocitycompensation_workmode[22]~[25]`，并在行1块内额外调用一次行2的 `update_OSD_chinese`。

---

## 调整布局注意事项

### 修改索引时

1. 每次移动一项，确认新索引范围不超过 [31]
2. 两个汉字字段必须连续（`_1` 在低索引，`_2` 在高索引）
3. 用 sed 批量替换时，注意多步替换的连锁问题（先把旧值→中间值→新值，避免中途被覆盖）

### 跨数组移动时

- 从行2移到行1：同时修改焦距/焦距触发块的数组名和索引，以及 update 调用
- 从行1移到行2：在行1触发块内写目标槽位后，需补调一次行2的 `update_OSD_chinese`

### 容量速算

```
汉字 N 个 → 2N 槽
ASCII M 字符 → M 槽
行1/行2 各 32 槽，已用见上表，剩余均为空槽
```

---

## 相关文件速查

| 文件 | 作用 |
|------|------|
| `main.cpp` ~4774 | 行1主写入块（传感器/视角/增强/放大/手动自动/红外极性） |
| `main.cpp` ~4974 | 行2主写入块（惯性/LMC/工作模式） |
| `main.cpp` ~5267 | 焦距值单独写入块 |
| `osd_graph_txt_chineseChar_app.cpp` ~54 | 数组初始值 |
| `osd_graph_txt_chineseChar_app.cpp` ~458 | 屏幕坐标配置（hor_x/ver_y） |
