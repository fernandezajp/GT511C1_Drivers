using System;
using Microsoft.SPOT;

namespace program
{
    public static class BitConverter
    {
        public static byte[] GetBytes(byte val)
        {
            return new byte[1] { val };
        }

        public static byte[] GetBytes(char val)
            {
            return new byte[2] { (byte)(val & 0xFF), (byte)((val >> 8) & 0xFF) };
            }

        public static byte[] GetBytes(short val)
        {
            return new byte[2] { (byte)(val & 0xFF), (byte)((val >> 8) & 0xFF) };
        }

        public static byte[] GetBytes(ushort val)
        {
            return new byte[2] { (byte)(val & 0xFF), (byte)((val >> 8) & 0xFF) };
        }

        public static byte[] GetBytes(int val)
        {
            return new byte[4] {
                (byte)(val & 0xFF),
                (byte)((val >> 8) & 0xFF),
                (byte)((val >> 16) & 0xFF),
                (byte)((val >> 24) & 0xFF) 
            };
        }

        public static byte[] GetBytes(uint val)
        {
            return new byte[4] {
                (byte)(val & 0xFF),
                (byte)((val >> 8) & 0xFF),
                (byte)((val >> 16) & 0xFF),
                (byte)((val >> 24) & 0xFF) 
            };
        }

        public static byte[] GetBytes(long val)
        {
        return new byte[8] {
                (byte)(val & 0xFF),
                (byte)((val >> 8 )& 0xFF),
                (byte)((val >> 16) & 0xFF),
                (byte)((val >> 24) & 0xFF),
                (byte)((val >> 32) & 0xFF),
                (byte)((val >> 40) & 0xFF),
                (byte)((val >> 48) & 0xFF),
                (byte)((val >> 56) & 0xFF)
            };
        }

        public static byte[] GetBytes(ulong val)
        {
            return new byte[8] {
                (byte)(val & 0xFF),
                (byte)((val >> 8) & 0xFF),
                (byte)((val >> 16) & 0xFF),
                (byte)((val >> 24) & 0xFF),
                (byte)((val >> 32) & 0xFF),
                (byte)((val >> 40) & 0xFF),
                (byte)((val >> 48) & 0xFF),
                (byte)((val >> 56) & 0xFF)
            };
        }

        //public static byte[] GetBytes(float val)
        //{
        //    byte[] buffer = new byte[4];
        //    GHIElectronics.NETMF.System.Util.InsertValueIntoArray(val, buffer, 0);
        //    return buffer;
        //}

        //public static byte[] GetBytes(double val)
        //{
        //    byte[] buffer = new byte[4];
        //    GHIElectronics.NETMF.System.Util.InsertValueIntoArray((float)val, buffer, 0);
        //    return buffer;
        //}

        public static char ToChar(byte[] val, int index)
        {
            return (char)(val[0 + index]);
        }
    }
}
