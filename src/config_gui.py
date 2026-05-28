#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
BSP 配置图形化工具
用于生成 bsp_config.h 配置文件
"""

import tkinter as tk
from tkinter import ttk, messagebox
import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
CONFIG_FILE = os.path.join(SCRIPT_DIR, "bsp_config.h")

class BSPConfigGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("BSP 配置工具 - HPM EtherCAT")
        self.root.geometry("600x700")
        
        self.config = {}
        self.load_default_config()
        self.create_widgets()
        
    def load_default_config(self):
        self.config = {
            "COM1_MODBUS_EN": 1,
            "COM1_BAUDRATE": 115200,
            "COM1_SLAVE_IDX": 1,
            "COM2_MODBUS_EN": 1,
            "COM2_BAUDRATE": 115200,
            "COM2_SLAVE_IDX": 1,
            "TCP_MODBUS_EN": 1,
            "TCP_SLAVE_IDX": 1,
            "TCP_IP_ADDR0": 192,
            "TCP_IP_ADDR1": 168,
            "TCP_IP_ADDR2": 1,
            "TCP_IP_ADDR3": 30,
            "TCP_PORT": 502,
            "DYNLIB_EN": 0,
            "AXIS_CYCLE_TIME": 1000000,
            "AXIS_NUM_PLUSE": 2,
            "AXIS_NUM_ETHERCAT": 0,
            "AXIS_NUM_CAN": 0,
            "ENCODER_NUM": 0,
            "ENCODER_NUM_ETHERCAT": 0,
            "MAIN_INPUT_NUM": 0,
            "MAIN_OUTPUT_NUM": 0,
            "ETHERCAT_IO_NUM": 0,
            "ETHERCAT_INPUT_NUM": [0] * 16,
            "ETHERCAT_OUTPUT_NUM": [0] * 16,
            "CAN_MASTER_EN": 0,
            "CAN_SLAVE_EN": 0,
            "CAN_SLAVE_NODE_ID": 1,
            "CAN_BITRATE": 1000,
            "FRAM_SAVE_PERIOD": 1000,
            "FRAM_AUTOSAVE_MAX_BYTES": 16000,
            "CFM_EN": 0,
        }
        
        if os.path.exists(CONFIG_FILE):
            self.parse_existing_config()
            
    def parse_existing_config(self):
        try:
            with open(CONFIG_FILE, 'r', encoding='utf-8') as f:
                content = f.read()
            for key in self.config:
                pattern = f"#define CFG_{key}"
                if pattern in content:
                    lines = content.split('\n')
                    for line in lines:
                        if line.startswith(pattern):
                            value_part = line[line.find(pattern) + len(pattern):].strip()
                            if value_part.startswith('{'):
                                value_str = value_part.strip('{}')
                                values = []
                                for v in value_str.split(','):
                                    v = v.strip()
                                    if v.isdigit():
                                        values.append(int(v))
                                if values:
                                    self.config[key] = values
                            else:
                                parts = line.split()
                                if len(parts) >= 3:
                                    value = parts[2].rstrip(';')
                                    if value == 'y' or value == '1':
                                        self.config[key] = 1
                                    elif value == 'n' or value == '0':
                                        self.config[key] = 0
                                    else:
                                        try:
                                            self.config[key] = int(value)
                                        except ValueError:
                                            pass
                            break

            # BITRATE 在头文件中无 CFG_ 前缀
            for line in content.split('\n'):
                stripped = line.strip()
                parts = stripped.split()
                if len(parts) >= 3 and parts[0] == "#define" and parts[1] == "BITRATE":
                    try:
                        self.config["CAN_BITRATE"] = int(parts[2])
                    except ValueError:
                        pass
                    break
        except Exception as e:
            print(f"读取现有配置失败: {e}")
                                        
    def create_widgets(self):
        notebook = ttk.Notebook(self.root)
        notebook.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        self.frames = {}
        
        # Modbus 通信配置
        frame_modbus = ttk.Frame(notebook)
        notebook.add(frame_modbus, text="Modbus通信")
        self.create_modbus_tab(frame_modbus)
        
        # 轴配置
        frame_axis = ttk.Frame(notebook)
        notebook.add(frame_axis, text="轴配置")
        self.create_axis_tab(frame_axis)
        
        # IO配置
        frame_io = ttk.Frame(notebook)
        notebook.add(frame_io, text="IO配置")
        self.create_io_tab(frame_io)
        
        # FRAM配置
        frame_fram = ttk.Frame(notebook)
        notebook.add(frame_fram, text="FRAM配置")
        self.create_fram_tab(frame_fram)
        
        # CAN配置
        frame_can = ttk.Frame(notebook)
        notebook.add(frame_can, text="CAN扩展")
        self.create_can_tab(frame_can)
        
        # 按钮区域
        btn_frame = ttk.Frame(self.root)
        btn_frame.pack(fill=tk.X, padx=10, pady=10)
        
        ttk.Button(btn_frame, text="生成配置文件", command=self.generate_config).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text="打开配置文件", command=self.open_config_file).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text="退出", command=self.root.quit).pack(side=tk.RIGHT, padx=5)
        
    def add_entry(self, parent, label, key, row, validate_cmd=None):
        ttk.Label(parent, text=label).grid(row=row, column=0, sticky=tk.W, padx=5, pady=3)
        var = tk.StringVar(value=str(self.config[key]))
        entry = ttk.Entry(parent, textvariable=var, width=20)
        entry.grid(row=row, column=1, sticky=tk.W, padx=5, pady=3)
        self.config[key] = var
        return var
        
    def add_checkbox(self, parent, label, key, row):
        var = tk.IntVar(value=self.config[key])
        ttk.Checkbutton(parent, text=label, variable=var).grid(row=row, column=0, columnspan=2, sticky=tk.W, padx=5, pady=3)
        self.config[key] = var
        return var
        
    def create_modbus_tab(self, parent):
        row = 0
        
        ttk.Label(parent, text="=== 串口1 (COM1) ===", font=('Arial', 10, 'bold')).grid(row=row, column=0, columnspan=2, sticky=tk.W, padx=5, pady=10)
        row += 1
        
        self.add_checkbox(parent, "使能 COM1 Modbus", "COM1_MODBUS_EN", row)
        row += 1
        self.add_entry(parent, "波特率:", "COM1_BAUDRATE", row)
        row += 1
        self.add_entry(parent, "从机地址:", "COM1_SLAVE_IDX", row)
        row += 1
        
        ttk.Label(parent, text="=== 串口2 (COM2) ===", font=('Arial', 10, 'bold')).grid(row=row, column=0, columnspan=2, sticky=tk.W, padx=5, pady=10)
        row += 1
        
        self.add_checkbox(parent, "使能 COM2 Modbus", "COM2_MODBUS_EN", row)
        row += 1
        self.add_entry(parent, "波特率:", "COM2_BAUDRATE", row)
        row += 1
        self.add_entry(parent, "从机地址:", "COM2_SLAVE_IDX", row)
        row += 1
        
        ttk.Label(parent, text="=== TCP Modbus ===", font=('Arial', 10, 'bold')).grid(row=row, column=0, columnspan=2, sticky=tk.W, padx=5, pady=10)
        row += 1
        
        self.add_checkbox(parent, "使能 TCP Modbus", "TCP_MODBUS_EN", row)
        row += 1
        self.add_entry(parent, "从机地址:", "TCP_SLAVE_IDX", row)
        row += 1
        
        ttk.Label(parent, text="IP地址:").grid(row=row, column=0, sticky=tk.W, padx=5, pady=3)
        ip_frame = ttk.Frame(parent)
        ip_frame.grid(row=row, column=1, sticky=tk.W, padx=5, pady=3)
        
        self.config["TCP_IP_ADDR0"] = tk.StringVar(value=str(self.config["TCP_IP_ADDR0"]))
        self.config["TCP_IP_ADDR1"] = tk.StringVar(value=str(self.config["TCP_IP_ADDR1"]))
        self.config["TCP_IP_ADDR2"] = tk.StringVar(value=str(self.config["TCP_IP_ADDR2"]))
        self.config["TCP_IP_ADDR3"] = tk.StringVar(value=str(self.config["TCP_IP_ADDR3"]))
        
        ttk.Entry(ip_frame, textvariable=self.config["TCP_IP_ADDR0"], width=5).pack(side=tk.LEFT)
        ttk.Label(ip_frame, text=".").pack(side=tk.LEFT)
        ttk.Entry(ip_frame, textvariable=self.config["TCP_IP_ADDR1"], width=5).pack(side=tk.LEFT)
        ttk.Label(ip_frame, text=".").pack(side=tk.LEFT)
        ttk.Entry(ip_frame, textvariable=self.config["TCP_IP_ADDR2"], width=5).pack(side=tk.LEFT)
        ttk.Label(ip_frame, text=".").pack(side=tk.LEFT)
        ttk.Entry(ip_frame, textvariable=self.config["TCP_IP_ADDR3"], width=5).pack(side=tk.LEFT)
        row += 1
        
        self.add_entry(parent, "端口:", "TCP_PORT", row)
        row += 1
        
        self.add_checkbox(parent, "使能 dynlib 0x40 扩展协议(仅TCP生效)", "DYNLIB_EN", row)
        
    def create_axis_tab(self, parent):
        row = 0
        
        self.add_entry(parent, "轴周期时间(us):", "AXIS_CYCLE_TIME", row)
        row += 1
        self.add_entry(parent, "脉冲轴数量:", "AXIS_NUM_PLUSE", row)
        row += 1
        self.add_entry(parent, "EtherCAT轴数量:", "AXIS_NUM_ETHERCAT", row)
        row += 1
        self.add_entry(parent, "本地编码器数量:", "ENCODER_NUM", row)
        row += 1
        self.add_entry(parent, "EtherCAT编码器数量:", "ENCODER_NUM_ETHERCAT", row)
        
    def create_io_tab(self, parent):
        row = 0
        
        self.add_entry(parent, "本地输入IO数量:", "MAIN_INPUT_NUM", row)
        row += 1
        self.add_entry(parent, "本地输出IO数量:", "MAIN_OUTPUT_NUM", row)
        row += 1
        
        ttk.Label(parent, text="EtherCAT IO卡数量:").grid(row=row, column=0, sticky=tk.W, padx=5, pady=3)
        self.io_num_var = tk.StringVar(value=str(self.config.get("ETHERCAT_IO_NUM", 0)))
        entry = ttk.Entry(parent, textvariable=self.io_num_var, width=20)
        entry.grid(row=row, column=1, sticky=tk.W, padx=5, pady=3)
        self.io_num_var.trace_add("write", lambda *args: self.update_io_entries())
        self.config["ETHERCAT_IO_NUM"] = self.io_num_var
        row += 1
        
        ttk.Label(parent, text="=== 每张IO卡输入输出数量 ===", font=('Arial', 10, 'bold')).grid(row=row, column=0, columnspan=2, sticky=tk.W, padx=5, pady=5)
        row += 1
        
        self.io_input_vars = []
        self.io_output_vars = []
        
        ethercat_input = self.config.get("ETHERCAT_INPUT_NUM", [0] * 16)
        ethercat_output = self.config.get("ETHERCAT_OUTPUT_NUM", [0] * 16)
        
        for i in range(16):
            input_val = str(ethercat_input[i]) if i < len(ethercat_input) else "0"
            output_val = str(ethercat_output[i]) if i < len(ethercat_output) else "0"
            input_var = tk.StringVar(value=input_val)
            output_var = tk.StringVar(value=output_val)
            self.io_input_vars.append(input_var)
            self.io_output_vars.append(output_var)
        
        self.io_frame = parent
        self.io_start_row = row
        self.update_io_entries()
        
    def update_io_entries(self):
        try:
            io_num = int(self.io_num_var.get())
        except:
            io_num = 0
        
        if io_num > 16:
            io_num = 16
            self.io_num_var.set("16")
        
        for widget in self.io_frame.grid_slaves():
            if int(widget.grid_info()["row"]) >= self.io_start_row:
                widget.destroy()
        
        row = self.io_start_row
        for i in range(io_num):
            ttk.Label(self.io_frame, text=f"IO卡{i+1} 输入:").grid(row=row, column=0, sticky=tk.W, padx=5, pady=2)
            ttk.Entry(self.io_frame, textvariable=self.io_input_vars[i], width=10).grid(row=row, column=1, sticky=tk.W, padx=5, pady=2)
            ttk.Label(self.io_frame, text=f"输出:").grid(row=row, column=2, sticky=tk.W, padx=5, pady=2)
            ttk.Entry(self.io_frame, textvariable=self.io_output_vars[i], width=10).grid(row=row, column=3, sticky=tk.W, padx=5, pady=2)
            row += 1
        
        self.config["ETHERCAT_IO_NUM"] = self.io_num_var
        
    def create_fram_tab(self, parent):
        row = 0

        self.add_entry(parent, "FRAM保存周期(ms):", "FRAM_SAVE_PERIOD", row)
        row += 1
        self.add_entry(parent, "FRAM自动保存最大字节数:", "FRAM_AUTOSAVE_MAX_BYTES", row)
        row += 1

        self.add_checkbox(parent, "CFM压力集采模块使能", "CFM_EN", row)

    def create_can_tab(self, parent):
        row = 0
        
        self.add_checkbox(parent, "使能 CAN主站", "CAN_MASTER_EN", row)
        row += 1
        self.add_checkbox(parent, "使能 CAN从站", "CAN_SLAVE_EN", row)
        row += 1
        self.add_entry(parent, "CAN从站节点号:", "CAN_SLAVE_NODE_ID", row)
        row += 1
        self.add_entry(parent, "CAN轴数量:", "AXIS_NUM_CAN", row)
        row += 1

        ttk.Label(parent, text="波特率:").grid(row=row, column=0, sticky=tk.W, padx=5, pady=3)
        self.can_bitrate_var = tk.StringVar(value=str(self.config["CAN_BITRATE"]))
        bitrate_combo = ttk.Combobox(parent, textvariable=self.can_bitrate_var,
                                      values=["250", "500", "1000"], width=17, state="readonly")
        bitrate_combo.grid(row=row, column=1, sticky=tk.W, padx=5, pady=3)
        self.config["CAN_BITRATE"] = self.can_bitrate_var
        row += 1
        
        bitrate_info = {
            "250": "250 Kbps",
            "500": "500 Kbps",
            "1000": "1000 Kbps (1 Mbps)",
        }
        self.can_bitrate_label = ttk.Label(parent, text=bitrate_info.get(self.can_bitrate_var.get(), ""))
        self.can_bitrate_label.grid(row=row, column=0, columnspan=2, sticky=tk.W, padx=5, pady=2)
        self.can_bitrate_var.trace_add("write", lambda *args: self.can_bitrate_label.config(
            text=bitrate_info.get(self.can_bitrate_var.get(), "")))
        
    def get_value(self, key):
        val = self.config[key]
        if isinstance(val, tk.IntVar):
            return val.get()
        elif isinstance(val, tk.StringVar):
            try:
                return int(val.get())
            except:
                return 0
        return val
    
    def get_io_input_str(self):
        try:
            io_num = int(self.io_num_var.get())
        except:
            io_num = 0
        values = []
        for i in range(io_num):
            try:
                values.append(str(int(self.io_input_vars[i].get())))
            except:
                values.append("0")
        for i in range(io_num, 16):
            values.append("0")
        return ", ".join(values)
    
    def get_io_output_str(self):
        try:
            io_num = int(self.io_num_var.get())
        except:
            io_num = 0
        values = []
        for i in range(io_num):
            try:
                values.append(str(int(self.io_output_vars[i].get())))
            except:
                values.append("0")
        for i in range(io_num, 16):
            values.append("0")
        return ", ".join(values)
        
    def generate_config(self):
        if self.get_value("CAN_MASTER_EN") and self.get_value("CAN_SLAVE_EN"):
            messagebox.showerror("错误", "CAN主站和CAN从站不能同时使能")
            return
        if self.get_value("CAN_SLAVE_EN") and not (1 <= self.get_value("CAN_SLAVE_NODE_ID") <= 31):
            messagebox.showerror("错误", "CAN从站节点号范围必须为 1..31")
            return

        v = self.get_value
        config_content = f'''/**
 * @file bsp_config.h
 * @brief BSP 用户配置头文件
 *
 * 该文件用于配置 BSP 参数，可通过以下方式修改：
 * 1. 直接编辑本文件
 * 2. 使用 Python 图形化配置工具: python config_gui.py
 *
 * 注意：修改后需要重新编译工程
 */

#ifndef BSP_CONFIG_H
#define BSP_CONFIG_H

/* ========== Modbus Communication ========== */
#define CFG_COM1_MODBUS_EN      {v("COM1_MODBUS_EN")}
#define CFG_COM1_BAUDRATE       {v("COM1_BAUDRATE")}
#define CFG_COM1_SLAVE_IDX      {v("COM1_SLAVE_IDX")}

#define CFG_COM2_MODBUS_EN      {v("COM2_MODBUS_EN")}
#define CFG_COM2_BAUDRATE       {v("COM2_BAUDRATE")}
#define CFG_COM2_SLAVE_IDX      {v("COM2_SLAVE_IDX")}

#define CFG_TCP_MODBUS_EN       {v("TCP_MODBUS_EN")}
#define CFG_TCP_SLAVE_IDX       {v("TCP_SLAVE_IDX")}
#define CFG_TCP_IP_ADDR0        {v("TCP_IP_ADDR0")}
#define CFG_TCP_IP_ADDR1        {v("TCP_IP_ADDR1")}
#define CFG_TCP_IP_ADDR2        {v("TCP_IP_ADDR2")}
#define CFG_TCP_IP_ADDR3        {v("TCP_IP_ADDR3")}
#define CFG_TCP_PORT            {v("TCP_PORT")}
#define CFG_DYNLIB_EN           {v("DYNLIB_EN")}          /* 是否使能 dynlib 0x40 扩展协议，仅对 TCP 生效 */

/* ========== Axis Configuration ========== */
#define CFG_AXIS_CYCLE_TIME     {v("AXIS_CYCLE_TIME")}    /* 轴周期时间(us) */
#define CFG_AXIS_NUM_PLUSE      {v("AXIS_NUM_PLUSE")}          /* 脉冲轴数量 */
#define CFG_AXIS_NUM_ETHERCAT   {v("AXIS_NUM_ETHERCAT")}          /* EtherCAT轴数量 */
#define CFG_ENCODER_NUM         {v("ENCODER_NUM")}          /* 本地编码器数量 */
#define CFG_ENCODER_NUM_ETHERCAT {v("ENCODER_NUM_ETHERCAT")}         /* EtherCAT编码器数量 */

/* ========== IO Configuration ========== */
#define CFG_MAIN_INPUT_NUM      {v("MAIN_INPUT_NUM")}          /* 本地输入IO数量 */
#define CFG_MAIN_OUTPUT_NUM     {v("MAIN_OUTPUT_NUM")}          /* 本地输出IO数量 */
#define CFG_ETHERCAT_IO_NUM     {v("ETHERCAT_IO_NUM")}          /* EtherCAT IO卡数量 */

/* EtherCAT IO卡输入输出数量 (最大16个卡) */
#define CFG_ETHERCAT_INPUT_NUM  {{{self.get_io_input_str()}}}
#define CFG_ETHERCAT_OUTPUT_NUM {{{self.get_io_output_str()}}}

/* ========== CAN Marster Configuration ========== */
#define CFG_CAN_MARSTER_EN      {v("CAN_MASTER_EN")}          /* CAN主站使能 */
/* ========== CAN Slave Configuration ========== */
#define CFG_CAN_SLAVE_EN        {v("CAN_SLAVE_EN")}          /* CAN从站使能 */
#define CFG_CAN_SLAVE_NODE_ID   {v("CAN_SLAVE_NODE_ID")}          /* CAN从站节点ID */
#define CFG_AXIS_NUM_CAN        {v("AXIS_NUM_CAN")}          /* CAN轴数量 */
/* ========== CAN Bitrate Configuration ========== */
#define BITRATE                 {v("CAN_BITRATE")}       /* CAN波特率，主从通用 */

/* ========== FRAM Configuration ========== */
#define CFG_FRAM_SAVE_PERIOD    {v("FRAM_SAVE_PERIOD")}       /* FRAM保存周期(ms) */
#define CFG_FRAM_AUTOSAVE_MAX_BYTES  {v("FRAM_AUTOSAVE_MAX_BYTES")} /* 周期性FRAM自动保存区最大字节数，须小于板载容量并避开OTA配置区 */
#define CFG_CFM_EN              {v("CFM_EN")}          /* CFM压力集采模块使能 */

#endif /* BSP_CONFIG_H */
'''
        
        try:
            os.makedirs(os.path.dirname(CONFIG_FILE), exist_ok=True)
            with open(CONFIG_FILE, 'w', encoding='utf-8') as f:
                f.write(config_content)
            messagebox.showinfo("成功", f"配置文件已生成:\n{os.path.abspath(CONFIG_FILE)}\n\n请重新编译工程")
        except Exception as e:
            messagebox.showerror("错误", f"生成配置文件失败:\n{str(e)}")
            
    def open_config_file(self):
        if os.path.exists(CONFIG_FILE):
            os.startfile(CONFIG_FILE)
        else:
            messagebox.showwarning("警告", "配置文件不存在，请先生成")

def main():
    root = tk.Tk()
    app = BSPConfigGUI(root)
    root.mainloop()

if __name__ == "__main__":
    main()
