#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ZKSW文件格式解码器 - 支持批量处理
针对中科世为FlyThings IDE的自定义格式
"""

import struct
import zlib
import os
import sys
import glob
from typing import Optional, Dict, Any, List, Tuple

class ZKSWDecoder:
    """ZKSW文件格式解码器"""
    
    def __init__(self, filepath: str):
        self.filepath = filepath
        self.data = None
        self.header_info = {}
        self.load_file()
    
    def load_file(self):
        """加载文件数据"""
        try:
            with open(self.filepath, 'rb') as f:
                self.data = f.read()
            print(f"成功加载文件: {self.filepath}, 大小: {len(self.data)} 字节")
        except Exception as e:
            print(f"加载文件失败: {e}")
            return False
        return True
    
    def verify_header(self) -> bool:
        """验证ZKSW文件头"""
        if len(self.data) < 16:
            print("文件太小，不是有效的ZKSW文件")
            return False
        
        # 检查文件头魔数
        magic = self.data[:4]
        if magic != b'ZKSW':
            print(f"无效的文件头魔数: {magic.hex()}")
            return False
        
        print("✓ 检测到有效的ZKSW文件头")
        return True
    
    def parse_header(self) -> Dict[str, Any]:
        """解析文件头信息"""
        if not self.verify_header():
            return {}
        
        # 解析前16字节的头部信息
        header_data = self.data[:16]
        
        magic = header_data[:4]
        value1 = struct.unpack('<I', header_data[4:8])[0]  # 小端序
        value2 = struct.unpack('<I', header_data[8:12])[0]
        value3 = struct.unpack('<I', header_data[12:16])[0]
        
        self.header_info = {
            'magic': magic.decode('ascii'),
            'value1': value1,
            'value1_hex': f'0x{value1:x}',
            'value2': value2, 
            'value2_hex': f'0x{value2:x}',
            'value3': value3,
            'value3_hex': f'0x{value3:x}'
        }
        
        print(f"文件头信息:")
        print(f"  魔数: {self.header_info['magic']}")
        print(f"  值1: {self.header_info['value1']} ({self.header_info['value1_hex']})")
        print(f"  值2: {self.header_info['value2']} ({self.header_info['value2_hex']})")
        print(f"  值3: {self.header_info['value3']} ({self.header_info['value3_hex']})")
        
        return self.header_info
    
    def find_compressed_data(self) -> Optional[int]:
        """查找可能的压缩数据起始位置"""
        # 查找ZLIB压缩头 (通常是0x78开头)
        zlib_signatures = [b'\x78\x9c', b'\x78\x01', b'\x78\xda', b'\x78\x5e']
        
        for i in range(16, len(self.data) - 2):
            for sig in zlib_signatures:
                if self.data[i:i+2] == sig:
                    print(f"在偏移 {i} 处发现可能的ZLIB压缩头: {sig.hex()}")
                    return i
        
        print("未找到明显的ZLIB压缩头")
        return None
    
    def try_decompress_zlib(self, offset: int = None) -> Optional[bytes]:
        """尝试ZLIB解压缩"""
        if offset is None:
            offset = self.find_compressed_data()
            if offset is None:
                offset = 16  # 默认跳过16字节头
        
        try:
            # 尝试从指定偏移开始解压
            compressed_data = self.data[offset:]
            decompressed = zlib.decompress(compressed_data)
            print(f"✓ ZLIB解压成功! 解压后大小: {len(decompressed)} 字节")
            return decompressed
        except zlib.error as e:
            print(f"ZLIB解压失败: {e}")
            
            # 尝试其他偏移位置
            for test_offset in [16, 32, 36, 64]:
                if test_offset < len(self.data):
                    try:
                        compressed_data = self.data[test_offset:]
                        decompressed = zlib.decompress(compressed_data)
                        print(f"✓ 在偏移 {test_offset} 处ZLIB解压成功! 解压后大小: {len(decompressed)} 字节")
                        return decompressed
                    except:
                        continue
            
            print("所有ZLIB解压尝试都失败")
            return None
    
    def try_xor_decrypt(self, data: bytes, keys: List[int] = None) -> Dict[int, bytes]:
        """尝试XOR解密"""
        if keys is None:
            keys = [0x00, 0x01, 0x42, 0x5a, 0xff, 0xaa, 0x55]
        
        results = {}
        
        for key in keys:
            try:
                decrypted = bytes(b ^ key for b in data[:100])  # 只处理前100字节用于测试
                results[key] = decrypted
                print(f"XOR密钥 0x{key:02x}: {self._safe_decode(decrypted)}")
            except Exception as e:
                print(f"XOR密钥 0x{key:02x} 解密失败: {e}")
        
        return results
    
    def _safe_decode(self, data: bytes, max_len: int = 50) -> str:
        """安全解码字节数据为可读字符串"""
        result = ""
        for i, b in enumerate(data[:max_len]):
            if 32 <= b <= 126:  # 可打印ASCII字符
                result += chr(b)
            else:
                result += f"\\x{b:02x}"
        return result + ("..." if len(data) > max_len else "")
    
    def extract_text_strings(self, min_length: int = 3) -> List[Tuple[int, str]]:
        """提取文件中的文本字符串"""
        strings = []
        current_string = ""
        start_offset = 0
        
        for i, byte in enumerate(self.data):
            if 32 <= byte <= 126:  # 可打印ASCII字符
                if not current_string:
                    start_offset = i
                current_string += chr(byte)
            else:
                if len(current_string) >= min_length:
                    strings.append((start_offset, current_string))
                current_string = ""
        
        # 处理末尾的字符串
        if len(current_string) >= min_length:
            strings.append((start_offset, current_string))
        
        return strings
    
    def analyze_structure(self) -> Dict[str, Any]:
        """分析文件结构"""
        analysis = {
            'file_size': len(self.data),
            'header_info': self.parse_header(),
            'compressed_data_offset': self.find_compressed_data(),
            'text_strings': self.extract_text_strings()
        }
        
        print(f"\n=== 文件结构分析 ===")
        print(f"文件大小: {analysis['file_size']} 字节")
        
        if analysis['text_strings']:
            print(f"\n找到 {len(analysis['text_strings'])} 个文本字符串:")
            for i, (offset, string) in enumerate(analysis['text_strings'][:10], 1):
                print(f"  {i}. [偏移:{offset}] \"{string}\"")
            if len(analysis['text_strings']) > 10:
                print(f"  ... 还有 {len(analysis['text_strings']) - 10} 个字符串")
        
        return analysis
    
    def decode(self, output_dir: str = None) -> bool:
        """主解码函数"""
        print(f"=== ZKSW文件解码器 ===")
        print(f"分析文件: {self.filepath}")
        
        if not self.data:
            print("文件加载失败")
            return False
        
        # 分析文件结构
        analysis = self.analyze_structure()
        
        # 尝试ZLIB解压
        print(f"\n=== ZLIB解压尝试 ===")
        decompressed_data = self.try_decompress_zlib()
        
        if decompressed_data:
            # 保存解压后的数据
            if output_dir:
                os.makedirs(output_dir, exist_ok=True)
                output_file = os.path.join(output_dir, "decompressed_data.bin")
                with open(output_file, 'wb') as f:
                    f.write(decompressed_data)
                print(f"解压数据已保存到: {output_file}")
                
                # 如果解压数据看起来像文本，也保存为文本文件
                try:
                    text_data = decompressed_data.decode('utf-8', errors='ignore')
                    if any(c.isprintable() for c in text_data):
                        text_file = os.path.join(output_dir, "decompressed_data.txt")
                        with open(text_file, 'w', encoding='utf-8') as f:
                            f.write(text_data)
                        print(f"文本数据已保存到: {text_file}")
                except:
                    pass
        
        # 尝试XOR解密
        print(f"\n=== XOR解密尝试 ===")
        raw_data = self.data[16:]  # 跳过头部
        xor_results = self.try_xor_decrypt(raw_data)
        
        return True


def expand_file_patterns(patterns: List[str]) -> List[str]:
    """展开文件模式，支持通配符"""
    all_files = []
    for pattern in patterns:
        # 支持通配符展开
        matched_files = glob.glob(pattern)
        if matched_files:
            all_files.extend(matched_files)
        elif os.path.exists(pattern):
            # 如果是具体文件路径
            all_files.append(pattern)
        else:
            print(f"警告: 未找到匹配的文件: {pattern}")
    
    return list(set(all_files))  # 去重


def main():
    """主函数 - 支持批量处理"""
    if len(sys.argv) < 2:
        print("用法: python zksw_decoder.py <文件1> [文件2] [文件3] ...")
        print("\n示例:")
        print("  单个文件:   python zksw_decoder.py sample.zksw")
        print("  多个文件:   python zksw_decoder.py file1.zksw file2.zksw file3.zksw")
        print("  使用通配符: python zksw_decoder.py *.zksw")
        print("  混合使用:   python zksw_decoder.py file1.zksw *.zksw data/*.zksw")
        return
    
    # 获取所有文件路径参数
    file_patterns = sys.argv[1:]
    
    # 展开通配符
    filepaths = expand_file_patterns(file_patterns)
    
    if not filepaths:
        print("错误: 没有找到任何文件")
        return
    
    print(f"找到 {len(filepaths)} 个文件待处理\n")
    
    # 统计结果
    success_count = 0
    failed_count = 0
    failed_files = []
    
    # 批量处理每个文件
    for i, filepath in enumerate(filepaths, 1):
        print(f"\n{'='*60}")
        print(f"处理文件 [{i}/{len(filepaths)}]: {filepath}")
        print(f"{'='*60}")
        
        if not os.path.exists(filepath):
            print(f"文件不存在: {filepath}")
            failed_count += 1
            failed_files.append(filepath)
            continue
        
        try:
            # 创建解码器
            decoder = ZKSWDecoder(filepath)
            
            # 创建输出目录
            output_dir = f"{filepath}_decoded"
            
            # 开始解码
            success = decoder.decode(output_dir)
            
            if success:
                print(f"✓ 文件解码成功: {filepath}")
                print(f"  结果保存在: {output_dir}")
                success_count += 1
            else:
                print(f"✗ 文件解码失败: {filepath}")
                failed_count += 1
                failed_files.append(filepath)
        
        except Exception as e:
            print(f"✗ 处理文件时出错: {filepath}")
            print(f"  错误信息: {e}")
            failed_count += 1
            failed_files.append(filepath)
    
    # 打印总结
    print(f"\n{'='*60}")
    print(f"=== 批量解码完成 ===")
    print(f"{'='*60}")
    print(f"总文件数: {len(filepaths)}")
    print(f"成功: {success_count}")
    print(f"失败: {failed_count}")
    
    if failed_files:
        print(f"\n失败的文件:")
        for f in failed_files:
            print(f"  - {f}")


if __name__ == "__main__":
    main()