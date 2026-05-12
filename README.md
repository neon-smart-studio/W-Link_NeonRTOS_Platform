<img width="1774" height="887" alt="image" src="https://github.com/user-attachments/assets/4cd7798b-5566-4ad4-9303-598c4cc4b297" />

W-Link 🚀

A Lightweight Hardware Abstraction Layer for STM32 (NeonRTOS Ready)

📌 Overview

W-Link 是一套為嵌入式系統設計的輕量級硬體抽象層（HAL-like framework），
專注於：

跨 STM32 全系列快速移植
高一致性 Driver API 設計
與 NeonRTOS / FreeRTOS / Bare-metal 無縫整合

目標是打造一個像 FreeRTOS + HAL 的組合，但更：
🔥 輕量
🔥 可控
🔥 適合商業客製專案

⚙️ Features
✅ 支援 STM32 多系列
✅ 統一 Driver 架構
✅ 可移植 SoC Layer
✅ 可整合 RTOS
✅ 高可維護性架構

W-Link 的設計理念：

寫一次 Driver，跨系列使用

傳統 STM32 開發常見問題：

不同系列 HAL API 不一致
Porting 成本高
Code fragmentation 嚴重

W-Link 解決：

統一 API
抽離 SoC 差異
讓上層邏輯完全不依賴 MCU 型號

## Third Party Components

This repository contains third-party middleware from STMicroelectronics.

Files under:
Bluetooth/BlueNRG/

remain the property of STMicroelectronics and retain their original license and copyright notices.

Only W-Link abstraction layers and framework code are licensed under Apache-2.0.