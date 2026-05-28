#include "Logic.h"
#include "Lock.h"
#include "vendor_sdk.h"

#define LOCKON              0xff
#define LOCKOFF             0xbb 
#define LOCKING             0xab 


ChipID IDST;

/**
 * @brief Vendor lock identifier.
 * @note Keep this value consistent with the corresponding app/tool configuration.
 */
const char firmSercet[]="VENDOR_PLACEHOLDER"; // Replace per deployment.
/// @brief 计算剩余时间
/// @param cur 当前日期
/// @param tar 目标日期
/// @return 剩余天数
int remaining_days(DateType cur,DateType tar)
{
    int y2, m2, d2;
    int y1, m1, d1;
    if(cur.day==0 || cur.day>31 || cur.month==0||cur.month>12)
    {
        return 0;
    }
    if(cur.day==0 || cur.day>31 || cur.month==0||cur.month>12)
    {
        return 0;
    }

    m1 = (cur.month + 9) % 12; //和3月1号的余数
    y1 = (cur.year+2000) - m1/10;     //必须是大于3月的才算完整年
    d1 = 365*y1 + y1/4 - y1/100 + y1/400 + (m1*306 + 5)/10 + (cur.day - 1);

    m2 = (tar.month + 9) % 12;
    y2 = (tar.year+2000) - m2/10;
    d2 = 365*y2 + y2/4 - y2/100 + y2/400 + (m2*306 + 5)/10 + (tar.day - 1);

    return (d2 - d1);
}



/**
* @brief 锁机
 *
 * @param
 */
int Logcheckin_MD5(void)
{
	//计算机器ID
	IDST = Get_ChipID();
	GSS.LockPara.LOCK_ID  = (IDST.id[0] % 100) * 10000;
	GSS.LockPara.LOCK_ID   += (IDST.id[1] % 100) * 100;
	
    //获取rtc时间
    DateType tar= {0};
    DateType cur = {0};
    int remainder_days=0;
    /******		日期判断	****/
	if(GSS.LockPara.Lockflag == LOCKOFF || GSS.LockPara.Lockflag == 0)   //永久解锁
    {
        remainder_days = 9999;
		GSS.LockPara.year = 2099;
    }
    else if(GSS.LockPara.Lockflag == LOCKING)//有时间锁
    {
        tar.year = GSS.LockPara.year - 2000;
        tar.month = GSS.LockPara.month;
        tar.day = GSS.LockPara.day;
        if(GSR.CurrTime.year < 2020)
        {
            remainder_days = -1;
            GSS.LockPara.Lockflag = LOCKON; //锁住了
        }
        else 
        {
            cur.year = GSR.CurrTime.year - 2000;
            cur.month = GSR.CurrTime.month;
            cur.day = GSR.CurrTime.day;
            remainder_days = remaining_days(cur,tar);
            if(remainder_days < 0)
            {
                GSS.LockPara.Lockflag = LOCKON; //锁住了
            }
        }
    }

    if (GSS.LockPara.Lockflag == LOCKON)
    {
        // AlarmSetBit(ALM3_Lock);  //锁机了
        remainder_days = 0;
    } else
    {
        // AlarmClearBit(ALM3_Lock);
    }
    /***********注册函数***************/
    int res = -1;
    if(GUW.Button.Unlock == 1)
    {
        GUW.Button.Unlock = 0;
        //剩余天数显示
		//读取板卡ID
        res = Md5Regist(GSS.LockPara.LOCK_ID, (char*)GSW.Decode);
		//手动输入ID
//		res = Md5Regist(GSS.LockPara.Define_ID, (char*)GSW.LockPara.decode);
        if(res>0)
        {
            GSS.LockPara.day = res%100; //20220628

            GSS.LockPara.year = res/10000+2000;

           GSS.LockPara.month = res/100%100;

            GSS.LockPara.Lockflag = LOCKING;
            if (GSS.LockPara.year >= 2050)
            {
                GSS.LockPara.Lockflag = LOCKOFF; //永久解锁
            }
            memset(&GSW.Decode,0,sizeof(GSW.Decode));
        }
    }
    return remainder_days;
}

/**
* @brief 锁机模块 特别注意:有一些卡是没有rtc时间的 调用时需要确保当前时间已经生效
 *
 * @param
 */
void LockCheckModul()
{
    if (GSR.CurrTime.year < 10)
    {
        return;
    }
    //触摸屏显示用
	//显示到期时间
    GUR.LockTime.year = GSS.LockPara.year;
    GUR.LockTime.month = GSS.LockPara.month;
    GUR.LockTime.day = GSS.LockPara.day;
	//显示剩余天数
    GUR.Remainder_days = Logcheckin_MD5();
}
