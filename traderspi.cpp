#include "StdAfx.h"
#include "traderspi.h"
#include "xTrader.h"
#include "xTraderDlg.h"
#include "NoticeDlg.h"
#include "BfTransfer.h"

#pragma warning(disable :4996)

extern HANDLE g_hEvent;
extern CWnd* g_pCWnd;
extern BOOL bRecconnect;

//������ϻָ������� �Զ�����
void CtpTraderSpi::OnFrontConnected()
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	if (g_pCWnd)
	{
		bRecconnect = TRUE;
		CXTraderDlg* pDlg = (CXTraderDlg*)g_pCWnd;
		//pDlg->SetStatusTxt( _T("TD��"),1);

		ReqUserLogin(pApp->m_sBROKER_ID,pApp->m_sINVESTOR_ID,pApp->m_sPASSWORD);
		DWORD dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
		if (dwRet==WAIT_OBJECT_0)
		{
			ResetEvent(g_hEvent);
			pDlg->SetTipTxt(_T("��������"),IDS_TRADE_TIPS);
			pDlg->SetPaneTxtColor(1,RED);

			SYSTEMTIME curTime;
			::GetLocalTime(&curTime);
			CString	szT;

			szT.Format(_T("%02d:%02d:%02d CTP�ص�¼�ɹ�"), curTime.wHour, curTime.wMinute, curTime.wSecond);
			pDlg->SetStatusTxt(szT,2);
		
		}
		else
			return;
	}
	
	else
	{
		ReqUserLogin(pApp->m_sBROKER_ID,pApp->m_sINVESTOR_ID,pApp->m_sPASSWORD);
	}

  //if (g_bOnce)SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqUserLogin(TThostFtdcBrokerIDType	vAppId,
	        TThostFtdcUserIDType	vUserId,	TThostFtdcPasswordType	vPasswd)
{
  
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, vAppId); strcpy(BROKER_ID, vAppId); 
	strcpy(req.UserID, vUserId);  strcpy(INVEST_ID, vUserId); 
	strcpy(req.Password, vPasswd);
	strcpy(req.UserProductInfo,PROD_INFO);
	//m_pDlg->ProgressUpdate(_T("��½����!"), 40);
	int iRet = pUserApi->ReqUserLogin(&req, ++m_iRequestID);
	//return iRet;
  //cerr<<" ���� | ���͵�¼..."<<((ret == 0) ? "�ɹ�" :"ʧ��") << endl;	
}

#define TIME_NULL "--:--:--"

void CtpTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if ( pRspInfo){memcpy(&m_RspMsg,pRspInfo,sizeof(CThostFtdcRspInfoField));}

	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin )
	{  
    // ����Ự����	
		m_ifrontId = pRspUserLogin->FrontID;
		m_isessionId = pRspUserLogin->SessionID;

		//strcpy(tradingday,pRspUserLogin->TradingDay);

		int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		sprintf(m_sOrdRef, "%d", ++nextOrderRef);

		SYSTEMTIME curTime;
		::GetLocalTime(&curTime);
		CTime tc(curTime);
		int i=0;
		int iHour[4],iMin[4],iSec[4];
		//MessageBoxA(NULL,pRspUserLogin->DCETime,"������ʱ��",MB_ICONINFORMATION);
		if (!strcmp(pRspUserLogin->DCETime,TIME_NULL) || !strcmp(pRspUserLogin->SHFETime,TIME_NULL))
		{
			
			for (i=0;i<4;i++)
			{
				iHour[i]=curTime.wHour;
				iMin[i]=curTime.wMinute;
				iSec[i]=curTime.wSecond;
			}
		}
		else
		{
			sscanf(pRspUserLogin->SHFETime, "%d:%d:%d", &iHour[0], &iMin[0], &iSec[0]);
			sscanf(pRspUserLogin->DCETime, "%d:%d:%d", &iHour[1], &iMin[1], &iSec[1]);
			sscanf(pRspUserLogin->CZCETime, "%d:%d:%d", &iHour[2], &iMin[2], &iSec[2]);
			sscanf(pRspUserLogin->FFEXTime, "%d:%d:%d", &iHour[3], &iMin[3], &iSec[3]);
		}
	
		CTime t[4];

		for (i=0;i<4;i++)
		{
			t[i] = CTime(curTime.wYear,curTime.wMonth,curTime.wDay,iHour[i],iMin[i],iSec[i]);
			m_tsEXnLocal[i] = t[i]-tc;
		}

		sprintf(m_sTmBegin,"%02d:%02d:%02d.%03d",curTime.wHour,curTime.wMinute,curTime.wSecond,curTime.wMilliseconds);
	    
	}
	//else
	//{memcpy(&g_RspMsg,pRspInfo,sizeof(CThostFtdcRspInfoField));}


  if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	pUserApi->ReqSettlementInfoConfirm(&req, ++m_iRequestID);
//	cerr<<" ���� | ���ͽ��㵥ȷ��..."<<((ret == 0)?"�ɹ�":"ʧ��")<<endl;
}

void CtpTraderSpi::OnRspSettlementInfoConfirm(
        CThostFtdcSettlementInfoConfirmField  *pSettlementInfoConfirm, 
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{	
	if( !IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm)
	{
    //cerr<<" ��Ӧ | ���㵥..."<<pSettlementInfoConfirm->InvestorID
     // <<"...<"<<pSettlementInfoConfirm->ConfirmDate
     // <<" "<<pSettlementInfoConfirm->ConfirmTime<<">...ȷ��"<<endl;
  }
  if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryNotice()
{
	CThostFtdcQryNoticeField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	pUserApi->ReqQryNotice(&req, ++m_iRequestID);
}

void CtpTraderSpi::ReqQrySettlementInfoConfirm()
{
	CThostFtdcQrySettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);

	pUserApi->ReqQrySettlementInfoConfirm(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) &&  pSettlementInfoConfirm)
	{
		//
	}
	
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQrySettlementInfo(TThostFtdcDateType TradingDay)
{
	CThostFtdcQrySettlementInfoField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	strcpy(req.TradingDay,TradingDay);

	pUserApi->ReqQrySettlementInfo(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) &&  pSettlementInfo)
	{
		CThostFtdcSettlementInfoField* pSi = new CThostFtdcSettlementInfoField();
		memcpy(pSi,pSettlementInfo,sizeof(CThostFtdcSettlementInfoField));
		m_StmiVec.push_back(pSi);
	}
	if(bIsLast) 
	{ 
		SetEvent(g_hEvent);
		
		SendNotifyMessage(HWND_BROADCAST,WM_QRYSMI_MSG,0,0);
		//Sleep(600);		
	}
}

void CtpTraderSpi::OnRspQryNotice(CThostFtdcNoticeField *pNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) &&  pNotice)
	{
		//CThostFtdcNoticeField* pNo = new CThostFtdcNoticeField();
		//TCHAR szNotice[10*MAX_PATH];
		//ansi2uni(CP_ACP,pNotice->Content,szNotice);
		//ShowErroTips(szNotice,MY_TIPS);
	}

	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryInst(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	if (instId != NULL)
	{ strcpy(req.InstrumentID, instId); }
	
	pUserApi->ReqQryInstrument(&req, ++m_iRequestID);
	//cerr<<" ���� | ���ͺ�Լ��ѯ..."<<((ret == 0)?"�ɹ�":"ʧ��")<<endl;
}

void CtpTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, 
         CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{ 
	//CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	if ( !IsErrorRspInfo(pRspInfo) &&  pInstrument)
	{
		PINSINFEX pInsInf = new INSINFEX;
		ZeroMemory(pInsInf,sizeof(INSINFEX));
		memcpy(pInsInf,  pInstrument, sizeof(INSTINFO));
		m_InsinfVec.push_back(pInsInf);
		//delete pInsInf;
	}
  if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryTdAcc()
{
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	pUserApi->ReqQryTradingAccount(&req, ++m_iRequestID);
	//cerr<<" ���� | �����ʽ��ѯ..."<<((ret == 0)?"�ɹ�":"ʧ��")<<endl;

}

void CtpTraderSpi::OnRspQryTradingAccount(
    CThostFtdcTradingAccountField *pTradingAccount, 
   CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{ 
	if (!IsErrorRspInfo(pRspInfo) &&  pTradingAccount)
	{
		 CThostFtdcTradingAccountField* pAcc = new CThostFtdcTradingAccountField();
		 memcpy(pAcc,pTradingAccount,sizeof(CThostFtdcTradingAccountField));
		 m_TdAcc = *pAcc ;

		if (g_pCWnd)
		{
			SendNotifyMessage(((CXTraderDlg*)g_pCWnd)->m_hWnd,WM_QRYACC_MSG,0,(LPARAM)pAcc);
			//memcpy(((CXTraderDlg*)g_pCWnd)->m_pTdAcc,pTradingAccount,sizeof(CThostFtdcTradingAccountField));
		}
		
	}
  if(bIsLast) SetEvent(g_hEvent);
}

//INSTRUMENT_ID��ɲ����ֶ�,����IF10,���ܲ������IF10��ͷ��ͷ��
void CtpTraderSpi::ReqQryInvPos(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}		
	pUserApi->ReqQryInvestorPosition(&req, ++m_iRequestID);
	//cerr<<" ���� | ���ͳֲֲ�ѯ..."<<((ret == 0)?"�ɹ�":"ʧ��")<<endl;
}

void CtpTraderSpi::OnRspQryInvestorPosition(
    CThostFtdcInvestorPositionField *pInvestorPosition, 
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{ 
  if( !IsErrorRspInfo(pRspInfo) &&  pInvestorPosition )
  {
	 CThostFtdcInvestorPositionField* pInvPos = new CThostFtdcInvestorPositionField();
	  memcpy(pInvPos,  pInvestorPosition, sizeof(CThostFtdcInvestorPositionField));
		m_InvPosVec.push_back(pInvPos);
  }
  if(bIsLast) SetEvent(g_hEvent);	
}

void CtpTraderSpi::ReqOrdLimit(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir,
	 TThostFtdcCombOffsetFlagType kpp,TThostFtdcPriceType price,   TThostFtdcVolumeType vol)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));	
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID); 
	strcpy(req.InstrumentID, instId); 	
	strcpy(req.OrderRef, m_sOrdRef);
	int nextOrderRef = atoi(m_sOrdRef);
	sprintf(m_sOrdRef, "%d", ++nextOrderRef);

	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;//�۸�����=�޼�	
	req.Direction = MapDirection(dir,true);  //��������	
	req.CombOffsetFlag[0] = MapOffset(kpp[0],true); //��Ͽ�ƽ��־:����
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	  //���Ͷ���ױ���־
	req.LimitPrice = price;	//�۸�
	req.VolumeTotalOriginal = vol;	///����	
	req.TimeCondition = THOST_FTDC_TC_GFD;  //��Ч������:������Ч
	req.VolumeCondition = THOST_FTDC_VC_AV; //�ɽ�������:�κ�����
	req.MinVolume = 1;	//��С�ɽ���:1	
	req.ContingentCondition = THOST_FTDC_CC_Immediately;  //��������:����
	
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//ǿƽԭ��:��ǿƽ	
	req.IsAutoSuspend = 0;  //�Զ������־:��	
	req.UserForceClose = 0;   //�û�ǿ����־:��

	pUserApi->ReqOrderInsert(&req, ++m_iRequestID);

}

///�м۵�
void CtpTraderSpi::ReqOrdAny(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,TThostFtdcVolumeType vol)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));	
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID); 
	strcpy(req.InstrumentID, instId); 	
	strcpy(req.OrderRef, m_sOrdRef);
	int nextOrderRef = atoi(m_sOrdRef);
	sprintf(m_sOrdRef, "%d", ++nextOrderRef);
	
	req.OrderPriceType = THOST_FTDC_OPT_AnyPrice;//�м�
	req.Direction = MapDirection(dir,true);  //��������	
	req.CombOffsetFlag[0] = MapOffset(kpp[0],true); //��Ͽ�ƽ��־:����
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	  //���Ͷ���ױ���־
	//req.LimitPrice = price;	//�۸�
	req.VolumeTotalOriginal = vol;	///����	
	req.TimeCondition = THOST_FTDC_TC_IOC;;  //������Ч
	req.VolumeCondition = THOST_FTDC_VC_AV; //�ɽ�������:�κ�����
	req.MinVolume = 1;	//��С�ɽ���:1	
	req.ContingentCondition = THOST_FTDC_CC_Immediately;  //��������:����
	
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//ǿƽԭ��:��ǿƽ	
	req.IsAutoSuspend = 0;  //�Զ������־:��	
	req.UserForceClose = 0;   //�û�ǿ����־:��
	
	pUserApi->ReqOrderInsert(&req, ++m_iRequestID);	
}

void CtpTraderSpi::ReqOrdCondition(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,
        TThostFtdcPriceType price,TThostFtdcVolumeType vol,TThostFtdcPriceType stopPrice,TThostFtdcContingentConditionType conType)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));	
	strcpy(req.BrokerID, BROKER_ID);  //���͹�˾����	
	strcpy(req.InvestorID, INVEST_ID); //Ͷ���ߴ���	
	strcpy(req.InstrumentID, instId); //��Լ����	
	strcpy(req.OrderRef, m_sOrdRef);  //��������
	int nextOrderRef = atoi(m_sOrdRef);
	sprintf(m_sOrdRef, "%d", ++nextOrderRef);
	
	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;	
	req.Direction = MapDirection(dir,true);  //��������	
	req.CombOffsetFlag[0] = MapOffset(kpp[0],true); //��Ͽ�ƽ��־:����
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	req.LimitPrice = price;	//�۸�
	req.VolumeTotalOriginal = vol;	///����	
	req.TimeCondition = THOST_FTDC_TC_GFD;  //������Ч
	req.VolumeCondition = THOST_FTDC_VC_AV; //�ɽ�������:�κ�����
	req.MinVolume = 1;	//��С�ɽ���:1	
	req.ContingentCondition = conType;  //��������
	
	req.StopPrice = stopPrice;  //ֹ���
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//ǿƽԭ��:��ǿƽ	
	req.IsAutoSuspend = 0;  //�Զ������־:��	
	req.UserForceClose = 0;   //�û�ǿ����־:��
	
	pUserApi->ReqOrderInsert(&req, ++m_iRequestID);
}

/*
FAK(Fill And Kill)ָ����ǽ���������Ч����ΪTHOST_FTDC_TC_IOC,ͬʱ,�ɽ���������ΪTHOST_FTDC_VC_AV,����������;
FOK(Fill Or Kill)ָ���ǽ���������Ч����������ΪTHOST_FTDC_TC_IOC,ͬʱ���ɽ�����������ΪTHOST_FTDC_VC_CV,��ȫ������.
����,��FAKָ����,����ָ����С�ɽ���,����ָ����λ��������С�ɽ��������ϳɽ�,ʣ�ඩ����ϵͳ����,����ϵͳȫ������.����״����,
��Ч����������ΪTHOST_FTDC_TC_IOC,����������ΪTHOST_FTDC_VC_MV,ͬʱ�趨MinVolume�ֶ�.
*/
void CtpTraderSpi::ReqOrdFAOK(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir,TThostFtdcCombOffsetFlagType kpp,
			TThostFtdcPriceType price,/*TThostFtdcVolumeType vol,*/TThostFtdcVolumeConditionType volconType,TThostFtdcVolumeType minVol)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));	
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID); 
	strcpy(req.InstrumentID, instId); 	
	strcpy(req.OrderRef, m_sOrdRef);
	int nextOrderRef = atoi(m_sOrdRef);
	sprintf(m_sOrdRef, "%d", ++nextOrderRef);
	
	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice; //�޼�
	req.Direction = MapDirection(dir,true);  //��������	
	req.CombOffsetFlag[0] = MapOffset(kpp[0],true); //��Ͽ�ƽ��־
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	//���Ͷ���ױ���־
	req.LimitPrice = price;	//�۸�
	//req.VolumeTotalOriginal = vol;	///����	
	req.TimeCondition = THOST_FTDC_TC_IOC;  //����
	req.VolumeCondition = volconType; //THOST_FTDC_VC_AV,THOST_FTDC_VC_MV;THOST_FTDC_VC_CV
	req.MinVolume = minVol;	//FAK��THOST_FTDC_VC_MVʱ���ָ��MinVol,���������0
	req.ContingentCondition = THOST_FTDC_CC_Immediately;  //��������:����
	
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//ǿƽԭ��:��ǿƽ	
	req.IsAutoSuspend = 0;  //�Զ������־:��	
	req.UserForceClose = 0;   //�û�ǿ����־:��
	
	pUserApi->ReqOrderInsert(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, 
          CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	CXTraderDlg* pDlg = (CXTraderDlg*)g_pCWnd;
	if( IsErrorRspInfo(pRspInfo) || (pInputOrder==NULL) )
	{
		TCHAR szErr[MAX_PATH];
		ansi2uni(CP_ACP,pRspInfo->ErrorMsg,szErr);
		
		/////////////////////////////////////////////////////////////////////
		CString szItems[ORDER_ITMES],szStat;

		szItems[0].Empty();
		ansi2uni(CP_ACP,pInputOrder->InstrumentID,szItems[1].GetBuffer(MAX_PATH));
		szItems[2]=JgBsType(pInputOrder->Direction);
		szItems[3]=JgOcType(pInputOrder->CombOffsetFlag[0]);
		
		szItems[4]=_GERR;
		
		szItems[5].Format(_T("%f"),pInputOrder->LimitPrice);
		szItems[5].TrimRight('0');
		int iLen = szItems[5].GetLength();
		if (szItems[5].Mid(iLen-1,1)==_T(".")) {szItems[5].TrimRight(_T("."));}
		
		szItems[6].Format(_T("%d"),pInputOrder->VolumeTotalOriginal);

		////////////////////////////////////////////////////////
		szStat.Format(_T("ʧ��:%s,%s,%s"),szItems[1],szItems[2],szItems[3]);
		pDlg->SetStatusTxt(szStat,2);
		//pDlg->SetPaneTxtColor(2,BLUE);
		////////////////////////////////////////////////////////

		szItems[7].Format(_T("%d"),pInputOrder->VolumeTotalOriginal);
		szItems[8] = _T("0");
		
		
		szItems[9] = szItems[5];
		getCurTime(szItems[10]);
		szItems[11] =  _T("0");
		szItems[12] =  _T("0");
		szItems[13]= szErr;
		
		pDlg->m_LstOrdInf.SetRedraw(FALSE);
		int nSubItem;
		COLORREF rTx=BLACK,rBg=WHITE;
		int nItem =pDlg->m_LstOrdInf.GetItemCount();

		for (nSubItem = 0; nSubItem < ORDER_ITMES; nSubItem++)
		{
			rBg = (nItem%2)?LITGRAY:WHITE;

			if (nSubItem == 0) {pDlg->m_LstOrdInf.InsertItem(0, NULL);}
			if (nSubItem == 2)
			{
				if (!_tcscmp(szItems[nSubItem],DIR_BUY))
				{ rTx = LITRED;}
				else
				{rTx = LITGREEN;}
			}
			
			else if (nSubItem == 4)
			{
				rTx=RED;		
			}
			else
			{
				rTx=BLACK;
			}
			
			pDlg->m_LstOrdInf.SetItemText(0, nSubItem, szItems[nSubItem], rTx,rBg);
			
		}
		pDlg->m_LstOrdInf.SetRedraw(TRUE);
		/////////////////////////////////////////////////////////////////////	
	}
  if(bIsLast) SetEvent(g_hEvent);	
}

void CtpTraderSpi::ReqOrderCancel(TThostFtdcSequenceNoType orderSeq)
{
  bool found=false; UINT i=0;
  for(i=0;i<m_orderVec.size();i++){
    if(m_orderVec[i]->BrokerOrderSeq == orderSeq){ found = true; break;}
  }
  if(!found)
  {
	  ////////�����ѱ��ɽ��򲻴���///////////
	  return;
  } 

	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.InvestorID, INVEST_ID); //Ͷ���ߴ���
	//strcpy(req.OrderRef, orderRef); //��������	
	//req.FrontID = frontId;           //ǰ�ñ��	
	//req.SessionID = sessionId;       //�Ự���
	strcpy(req.ExchangeID, m_orderVec[i]->ExchangeID);
	strcpy(req.OrderSysID, m_orderVec[i]->OrderSysID);
	req.ActionFlag = THOST_FTDC_AF_Delete;  //������־ 

	pUserApi->ReqOrderAction(&req, ++m_iRequestID);
	//cerr<< " ���� | ���ͳ���..." <<((ret == 0)?"�ɹ�":"ʧ��") << endl;
}

void CtpTraderSpi::OnRspOrderAction(
      CThostFtdcInputOrderActionField *pInputOrderAction, 
      CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{	
  if (IsErrorRspInfo(pRspInfo) || (pInputOrderAction==NULL))
  {
	 if (g_pCWnd)
	{
	 //////////////////////////////////////////
		CXTraderDlg* pDlg = (CXTraderDlg*)g_pCWnd;

		pDlg->SetStatusTxt(_T("����ʧ��!"),2);
	 }

  }
  if(bIsLast) SetEvent(g_hEvent);	
}

///�����ر�
void CtpTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{	
  CThostFtdcOrderField* order = new CThostFtdcOrderField();
  memcpy(order,  pOrder, sizeof(CThostFtdcOrderField));
  bool founded=false;    UINT i=0;
  for(i=0; i<m_orderVec.size(); i++)
  {
    if(m_orderVec[i]->BrokerOrderSeq == order->BrokerOrderSeq) 
	{ founded=true;    break;}
  }
  //////�޸��ѷ����ı���״̬
  if(founded) 
  {
	m_orderVec[i]= order; 

	if (g_pCWnd && !bRecconnect)
	{
		//////////////////////////////////////////////////////////
		CXTraderDlg* pDlg = (CXTraderDlg*)g_pCWnd;

		pDlg->InsertLstOrder(order,i+1);

		///////////////////////////ˢ�¹ҵ��б�/////////////////////
		int nRet = pDlg->FindOrdInOnRoadVec(order->BrokerOrderSeq);
		//δ����ҵ��б��
		if (nRet==-1)
		{
			//�����б�
			if (order->OrderStatus == '1' || order->OrderStatus == '3')
			{
				pDlg->m_onRoadVec.push_back(order);
				pDlg->InsertLstOnRoad(order,0,true);
			}
		}
		else
		{
			//�Ѿ����б�� ������ ��ɾ��
			if (order->OrderStatus != '1' && order->OrderStatus != '3')
			{
				pDlg->m_onRoadVec.erase(pDlg->m_onRoadVec.begin()+nRet);
				int nRet2 = pDlg->FindOrdInOnRoadLst(order->OrderSysID);
				
				if (nRet2 !=-1)
				{
					pDlg->m_LstOnRoad.SetRedraw(FALSE);
					pDlg->m_LstOnRoad.DeleteItem(nRet2);
					pDlg->m_LstOnRoad.SetRedraw(TRUE);
				}
					
			}
			else
			{
				//���ڵĹҵ� �޸�״̬
				int nRet2 = pDlg->FindOrdInOnRoadLst(order->OrderSysID);
				pDlg->InsertLstOnRoad(order,nRet2,false);
			}

		}
		/////////////////////////////////////////////////////////
	}

  } 
  ///////������ί�е�
  else 
  {
	m_orderVec.push_back(order);
	if (g_pCWnd)
	{
		//////////////////////////////////////////////////////////
		CXTraderDlg* pDlg = (CXTraderDlg*)g_pCWnd;

		pDlg->InsertLstOrder(order,0);
		
		/*
		if (order->OrderStatus == '1' || order->OrderStatus == '3')
		{
			pDlg->m_onRoadVec.push_back(order);
			pDlg->InsertLstOnRoad(order,0,true);
		}
		*/

	}
  }

  SetEvent(g_hEvent);
}

///�ɽ�֪ͨ
void CtpTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
  CThostFtdcTradeField* trade = new CThostFtdcTradeField();
  memcpy(trade,  pTrade, sizeof(CThostFtdcTradeField));
  bool founded=false;     unsigned int i=0;
  for(i=0; i<m_tradeVec.size(); i++){
    if(!strcmp(m_tradeVec[i]->TradeID,trade->TradeID)) {
      founded=true;   break;
    }
  }
  //////�޸ĳɽ���״̬
  if(founded) 
  {
	  m_tradeVec[i]= trade; 
	  
	  if (g_pCWnd && !bRecconnect)
	  {
		  CXTraderDlg* pDlg = (CXTraderDlg*)g_pCWnd;
		  pDlg->InsertLstTrade(trade,i+1);  
	  } 
  }
  ///////�������ѳɽ��� 
  else 
  {
	m_tradeVec.push_back(trade);
	if (g_pCWnd)
	{
		CXTraderDlg* pDlg = (CXTraderDlg*)g_pCWnd;
				  
		pDlg->InsertLstTrade(trade,0);
	///////////////////////////////////////////////////////////	  
	}
 
  }
  //cerr<<" �ر� | �����ѳɽ�...�ɽ����:"<<trade->TradeID<<endl;
  SetEvent(g_hEvent);
}

void CtpTraderSpi::OnFrontDisconnected(int nReason)
{
	if (g_pCWnd)
	{
		CXTraderDlg* pDlg = (CXTraderDlg*)g_pCWnd;
		//pDlg->SetStatusTxt(_T("TD��"),1);
		pDlg->SetTipTxt(_T("���׶Ͽ�"),IDS_TRADE_TIPS);
		pDlg->SetPaneTxtColor(1,BLUE);

		SYSTEMTIME curTime;
		::GetLocalTime(&curTime);
		CString	szT;
			
		
		szT.Format(_T("%02d:%02d:%02d CTP�жϵȴ�����"), curTime.wHour, curTime.wMinute, curTime.wSecond);
		 pDlg->SetStatusTxt(szT,2);
		ShowErroTips(IDS_DISCONTIPS,IDS_STRTIPS);

	}
}
		
void CtpTraderSpi::OnHeartBeatWarning(int nTimeLapse)
{
	cerr<<" ��Ӧ | ������ʱ����..." 
	  << " TimerLapse = " << nTimeLapse << endl;
}

///�����ѯ���ױ���
void CtpTraderSpi::ReqQryTradingCode()
{

	CThostFtdcQryTradingCodeField req;
	memset(&req, 0, sizeof(req));

	req.ClientIDType = THOST_FTDC_CIDT_Speculation;

	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.InvestorID, INVEST_ID); //Ͷ���ߴ���

	pUserApi->ReqQryTradingCode(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pTradingCode )
	{
		CThostFtdcTradingCodeField* pTdCode = new CThostFtdcTradingCodeField();
		memcpy(pTdCode, pTradingCode, sizeof(CThostFtdcTradingCodeField));
		m_TdCodeVec.push_back(pTdCode);
	}
  if(bIsLast) SetEvent(g_hEvent);
}

///�����ѯ��Լ��֤����
void CtpTraderSpi::ReqQryInstMgr(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentMarginRateField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.InvestorID, INVEST_ID); //Ͷ���ߴ���
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}	
	req.HedgeFlag = '1';
	pUserApi->ReqQryInstrumentMarginRate(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pInstrumentMarginRate )
	{

		CThostFtdcInstrumentMarginRateField* pMaginRate = new CThostFtdcInstrumentMarginRateField();
		memcpy(pMaginRate,  pInstrumentMarginRate, sizeof(CThostFtdcInstrumentMarginRateField));
		m_MargRateVec.push_back(pMaginRate);
	}
  if(bIsLast) SetEvent(g_hEvent);

}

///�����ѯ��Լ��������
void CtpTraderSpi::ReqQryInstFee(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentCommissionRateField req;
	
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.InvestorID, INVEST_ID); //Ͷ���ߴ���
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}	
	pUserApi->ReqQryInstrumentCommissionRate(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pInstrumentCommissionRate )
	{
		//CThostFtdcInstrumentCommissionRateField* pFeeRate = new CThostFtdcInstrumentCommissionRateField();
		memcpy(&m_FeeRateRev,  pInstrumentCommissionRate, sizeof(CThostFtdcInstrumentCommissionRateField)); 
		//FeeRateList.push_back(pFeeRate);
		
	}
  if(bIsLast) SetEvent(g_hEvent);

}

//////////////////�����ѯ�û�����/////////////
void CtpTraderSpi::ReqQryInvestor()
{
	CThostFtdcQryInvestorField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.InvestorID, INVEST_ID); //Ͷ���ߴ���

	pUserApi->ReqQryInvestor(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryInvestor(CThostFtdcInvestorField *pInvestor, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pInvestor )
	{
		CThostFtdcInvestorField *pInv = new CThostFtdcInvestorField;
		memcpy(pInv,pInvestor,sizeof(CThostFtdcInvestorField));

		SendNotifyMessage(((CXTraderDlg*)g_pCWnd)->m_hWnd,WM_QRYUSER_MSG,0,(LPARAM)pInv);
	}
  if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqUserPwdUpdate(TThostFtdcPasswordType szNewPass,TThostFtdcPasswordType szOldPass)
{
	CThostFtdcUserPasswordUpdateField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.UserID, INVEST_ID); //�û�����
	strcpy(req.NewPassword, szNewPass);    	
	strcpy(req.OldPassword,szOldPass);  

	pUserApi->ReqUserPasswordUpdate(&req,++m_iRequestID);
}

//�ʽ��˻�����
void CtpTraderSpi::ReqTdAccPwdUpdate(TThostFtdcPasswordType szNewPass,TThostFtdcPasswordType szOldPass)
{
	CThostFtdcTradingAccountPasswordUpdateField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.AccountID, INVEST_ID); //�û�����
	strcpy(req.NewPassword, szNewPass);    	
	strcpy(req.OldPassword,szOldPass);  

	pUserApi->ReqTradingAccountPasswordUpdate(&req,++m_iRequestID);
}

///�û��������������Ӧ
void CtpTraderSpi::OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pUserPasswordUpdate )
	{
		//memcpy(&g_szUserPass,pUserPasswordUpdate->NewPassword,sizeof(CThostFtdcUserPasswordUpdateField));
		//AfxMessageBox(tName);
	}
  if(bIsLast) SetEvent(g_hEvent);
}
	
///�ʽ��˻��������������Ӧ
void CtpTraderSpi::OnRspTradingAccountPasswordUpdate(CThostFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pTradingAccountPasswordUpdate )
	{
		//memcpy(&g_szAccPass,pTradingAccountPasswordUpdate->NewPassword,sizeof(CThostFtdcTradingAccountPasswordUpdateField));
		//AfxMessageBox(tName);
	}
  if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqAuthenticate(TThostFtdcProductInfoType UserProdInf,TThostFtdcAuthCodeType	AuthCode)
{
	CThostFtdcReqAuthenticateField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.UserID, INVEST_ID); //�û�����
	strcpy(req.UserProductInfo, UserProdInf);    	
	strcpy(req.AuthCode,AuthCode);  

	pUserApi->ReqAuthenticate(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pRspAuthenticateField )
	{
		//memcpy(&g_InvInf,pInvestor,sizeof(CThostFtdcInvestorField));
		//AfxMessageBox(tName);
	}
  if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryAccreg()
{
	CThostFtdcQryAccountregisterField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.AccountID, INVEST_ID); //�û�����

	pUserApi->ReqQryAccountregister(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryAccountregister(CThostFtdcAccountregisterField *pAccountregister, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pAccountregister)
	{
		CThostFtdcAccountregisterField* pAccReg = new CThostFtdcAccountregisterField();
		memcpy(pAccReg,  pAccountregister, sizeof(CThostFtdcAccountregisterField));
		m_AccRegVec.push_back(pAccReg);

	}
  if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryTransBk(TThostFtdcBankIDType BankID,TThostFtdcBankBrchIDType BankBrchID)
{
	CThostFtdcQryTransferBankField req;
	memset(&req, 0, sizeof(req));


	pUserApi->ReqQryTransferBank(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryTransferBank(CThostFtdcTransferBankField *pTransferBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pTransferBank)
	{
		//memcpy(&g_InvInf,pInvestor,sizeof(CThostFtdcInvestorField));
		//AfxMessageBox(tName);
	}
  if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryContractBk()
{
	CThostFtdcQryContractBankField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	

	pUserApi->ReqQryContractBank(&req,++m_iRequestID);
}

  void CtpTraderSpi::OnRspQryContractBank(CThostFtdcContractBankField *pContractBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
 {
	  if( !IsErrorRspInfo(pRspInfo) && pContractBank)
	  {
		  //TCHAR szBankID[20],szBankBrchID[20],szBankName[MAX_PATH];
		  	//ansi2uni(CP_ACP,pContractBank->BankID,szBankID);
			//ansi2uni(CP_ACP,pContractBank->BankBrchID,szBankBrchID);
			//ansi2uni(CP_ACP,pContractBank->BankName,szBankName);

			//CString str;
			//str.Format(_T("BankID: %s; BankBrchID: %s; BankName: %s"),szBankID,szBankBrchID,szBankName);
		  //memcpy(&g_InvInf,pInvestor,sizeof(CThostFtdcInvestorField));
		  //AfxMessageBox(str);
	  }
	if(bIsLast) SetEvent(g_hEvent);
 }

  //////////////////////////////////////////�ڻ����������ʽ�ת�ڻ�����///////////////////////////////////////
void CtpTraderSpi::ReqBk2FByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,
	 TThostFtdcPasswordType Pwd,TThostFtdcTradeAmountType TdAmt)
{
	CThostFtdcReqTransferField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	 
	strcpy(req.AccountID, INVEST_ID); //�û�����
	strcpy(req.TradeCode,"202001");
	strcpy(req.BankBranchID,"0000");
	strcpy(req.CurrencyID,"RMB");
	strcpy(req.BankID,BkID);
	strcpy(req.BankPassWord,BkPwd);
	strcpy(req.Password,Pwd);
	req.TradeAmount=TdAmt;
	req.SecuPwdFlag = THOST_FTDC_BPWDF_BlankCheck;

	pUserApi->ReqFromBankToFutureByFuture(&req,++m_iRequestID);
}
 
 ///�ڻ����������ʽ�ת�ڻ�Ӧ��
void CtpTraderSpi::OnRspFromBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( IsErrorRspInfo(pRspInfo) || (pReqTransfer==NULL))
	{
		TCHAR szMsg[MAX_PATH];
		ansi2uni(CP_ACP,pRspInfo->ErrorMsg,szMsg);
		
		ShowErroTips(szMsg,MY_TIPS);
	}
	if(bIsLast) SetEvent(g_hEvent);	 
}
///�ڻ����������ʽ�ת�ڻ�֪ͨ
void CtpTraderSpi::OnRtnFromBankToFutureByFuture(CThostFtdcRspTransferField *pRspTransfer)
{
  CThostFtdcRspTransferField* bfTrans = new CThostFtdcRspTransferField();
  memcpy(bfTrans,  pRspTransfer, sizeof(CThostFtdcRspTransferField));
  bool founded=false;    UINT i=0;
  for(i=0; i<m_BfTransVec.size(); i++)
  {
    if(!strcmp(m_BfTransVec[i]->BankSerial,bfTrans->BankSerial)) 
	{ founded=true;    break;}
  }
  //////����
  if(founded) 
  {
	  m_BfTransVec[i]= bfTrans; 
  } 
  ///////�������ڷ���
  else 
  {
	  m_BfTransVec.push_back(bfTrans);
	  if(g_pCWnd && !bRecconnect)
	  {
		  if(pRspTransfer->ErrorID!=0)
		  {
			  TCHAR szMsg[MAX_PATH];
			  ansi2uni(CP_ACP,pRspTransfer->ErrorMsg,szMsg);
			  
			  ShowErroTips(szMsg,MY_TIPS);
		  }
		  else
		  {
			  ShowErroTips(IDS_BFTRANS_OK,IDS_STRTIPS);
		  }
	  }
	  
  }

  //SetEvent(g_hEvent);
}

///�ڻ����������ʽ�ת�ڻ�����ر�
void CtpTraderSpi::OnErrRtnBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo)
{

}


///////////////////////////////////////////�ڻ������ڻ��ʽ�ת��������///////////////////////////////////////////
void CtpTraderSpi::ReqF2BkByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,
	 TThostFtdcPasswordType Pwd,TThostFtdcTradeAmountType TdAmt)
 {
	 CThostFtdcReqTransferField req;
	 memset(&req, 0, sizeof(req));
	 
	 strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	 
	 strcpy(req.AccountID, INVEST_ID); //�û�����
	 strcpy(req.TradeCode,"202002");
	 strcpy(req.BankBranchID,"0000");
	 strcpy(req.CurrencyID,"RMB");
	 strcpy(req.BankID,BkID);
	 strcpy(req.BankPassWord,BkPwd);
	 strcpy(req.Password,Pwd);
	 req.TradeAmount=TdAmt;
	 req.SecuPwdFlag = THOST_FTDC_BPWDF_BlankCheck;
	 
	 pUserApi->ReqFromFutureToBankByFuture(&req,++m_iRequestID);
 }


 ///�ڻ������ڻ��ʽ�ת����Ӧ��
 void CtpTraderSpi::OnRspFromFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
 {
	 if( IsErrorRspInfo(pRspInfo) || (pReqTransfer==NULL))
	 {
		TCHAR szMsg[MAX_PATH];
		ansi2uni(CP_ACP,pRspInfo->ErrorMsg,szMsg);
		 
		ShowErroTips(szMsg,MY_TIPS);
	 }

	 if(bIsLast) SetEvent(g_hEvent);	
 }

 
///�ڻ������ڻ��ʽ�ת����֪ͨ
void CtpTraderSpi::OnRtnFromFutureToBankByFuture(CThostFtdcRspTransferField *pRspTransfer)
{
  CThostFtdcRspTransferField* bfTrans = new CThostFtdcRspTransferField();
  memcpy(bfTrans,  pRspTransfer, sizeof(CThostFtdcRspTransferField));
  bool founded=false;    UINT i=0;
  for(i=0; i<m_BfTransVec.size(); i++)
  {
	 if(!strcmp(m_BfTransVec[i]->BankSerial,bfTrans->BankSerial))
	  { founded=true;    break;}
  }
  //////����
  if(founded) 
  {
	  m_BfTransVec[i]= bfTrans; 
	  
  } 
  ///////�������ڷ���
  else 
  {
	  m_BfTransVec.push_back(bfTrans);
	  if(g_pCWnd && !bRecconnect)
	  {
		  if(pRspTransfer->ErrorID!=0)
		  {
			  TCHAR szMsg[MAX_PATH];
			  ansi2uni(CP_ACP,pRspTransfer->ErrorMsg,szMsg);
			  
			  ShowErroTips(szMsg,MY_TIPS);
		  }
		  else
		  {
			  ShowErroTips(IDS_BFTRANS_OK,IDS_STRTIPS);
		  }
	  }
  }
  
  //SetEvent(g_hEvent);
}



///�ڻ������ڻ��ʽ�ת���д���ر�
void CtpTraderSpi::OnErrRtnFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo)
{

}


 ///////////////////////////////////////////////////�ڻ������ѯ�����������///////////////////////////////////////////////
void CtpTraderSpi::ReqQryBkAccMoneyByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,
	 TThostFtdcPasswordType Pwd)
{
	CThostFtdcReqQueryAccountField req;
	memset(&req, 0, sizeof(req));
	
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	 
	strcpy(req.AccountID, INVEST_ID); //�û�����
	strcpy(req.TradeCode,"204002");
	strcpy(req.BankBranchID,"0000");
	strcpy(req.CurrencyID,"RMB");
	strcpy(req.BankID,BkID);
	strcpy(req.BankPassWord,BkPwd);
	strcpy(req.Password,Pwd);

	req.SecuPwdFlag = THOST_FTDC_BPWDF_BlankCheck;
	pUserApi->ReqQueryBankAccountMoneyByFuture(&req,++m_iRequestID);
}

///�ڻ������ѯ�������Ӧ��
void CtpTraderSpi::OnRspQueryBankAccountMoneyByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	
	if( IsErrorRspInfo(pRspInfo) || (pReqQueryAccount==NULL))
	{
		//TCHAR szMsg[MAX_PATH];
		//ansi2uni(CP_ACP,pRspInfo->ErrorMsg,szMsg);
		
		//ShowErroTips(szMsg,MY_TIPS);
	}
	//if(bIsLast) SetEvent(g_hEvent);	
	
}

///�ڻ������ѯ�������֪ͨ
void CtpTraderSpi::OnRtnQueryBankBalanceByFuture(CThostFtdcNotifyQueryAccountField *pNotifyQueryAccount)
{
	if(pNotifyQueryAccount->ErrorID ==0)
	{
		if (g_pCWnd && !bRecconnect)
		{

			CThostFtdcNotifyQueryAccountField *pNotify = new CThostFtdcNotifyQueryAccountField();
			memcpy(pNotify,pNotifyQueryAccount,sizeof(CThostFtdcNotifyQueryAccountField));

			::PostMessage(g_pCWnd->m_hWnd,WM_QRYBKYE_MSG,0,(LPARAM)pNotify);
			/*
			HWND hwnd = g_pCWnd->m_hWnd;
			
			////////////////////////////////////
			{
				COPYDATASTRUCT cpd;
				cpd.dwData = 0x10000;		//��ʶ
				cpd.cbData = sizeof(CThostFtdcNotifyQueryAccountField);
				cpd.lpData = (PVOID)pNotify;
				::SendMessage( hwnd, WM_COPYDATA, NULL, (LPARAM)&cpd );
			}
			*/

		}

	}

	SetEvent(g_hEvent);	
}


///�ڻ������ѯ����������ر�
void CtpTraderSpi::OnErrRtnQueryBankBalanceByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, CThostFtdcRspInfoField *pRspInfo)
{

}

///////////////////////////////////////��ѯת����ˮ////////////////////////////////////////////
/// "204005"
void CtpTraderSpi::ReqQryTfSerial(TThostFtdcBankIDType BkID)
{
	CThostFtdcQryTransferSerialField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	 
	strcpy(req.AccountID, INVEST_ID); //�û�����

	strcpy(req.BankID,BkID);

	pUserApi->ReqQryTransferSerial(&req,++m_iRequestID);
}
///�����ѯת����ˮ��Ӧ
void CtpTraderSpi::OnRspQryTransferSerial(CThostFtdcTransferSerialField *pTransferSerial, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( IsErrorRspInfo(pRspInfo) || (pTransferSerial==NULL))
	{

		
	}
	//if(bIsLast) SetEvent(g_hEvent);	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
///ϵͳ����ʱ�ڻ����ֹ������������ת�ڻ��������д�����Ϻ��̷��ص�֪ͨ
void CtpTraderSpi::OnRtnRepealFromBankToFutureByFutureManual(CThostFtdcRspRepealField *pRspRepeal)
{
}


///ϵͳ����ʱ�ڻ����ֹ���������ڻ�ת�����������д�����Ϻ��̷��ص�֪ͨ
void CtpTraderSpi::OnRtnRepealFromFutureToBankByFutureManual(CThostFtdcRspRepealField *pRspRepeal)
{
}


///ϵͳ����ʱ�ڻ����ֹ������������ת�ڻ�����ر�
void CtpTraderSpi::OnErrRtnRepealBankToFutureByFutureManual(CThostFtdcReqRepealField *pReqRepeal, CThostFtdcRspInfoField *pRspInfo)
{

}

///ϵͳ����ʱ�ڻ����ֹ���������ڻ�ת���д���ر�
void CtpTraderSpi::OnErrRtnRepealFutureToBankByFutureManual(CThostFtdcReqRepealField *pReqRepeal, CThostFtdcRspInfoField *pRspInfo)
{
}


///�ڻ������������ת�ڻ��������д�����Ϻ��̷��ص�֪ͨ
void CtpTraderSpi::OnRtnRepealFromBankToFutureByFuture(CThostFtdcRspRepealField *pRspRepeal)
{
}


///�ڻ���������ڻ�ת�����������д�����Ϻ��̷��ص�֪ͨ
void CtpTraderSpi::OnRtnRepealFromFutureToBankByFuture(CThostFtdcRspRepealField *pRspRepeal)
{
}



void CtpTraderSpi::ReqQryCFMMCTdAccKey()
{
	CThostFtdcQryCFMMCTradingAccountKeyField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	 
	strcpy(req.InvestorID, INVEST_ID); //�û�����

	pUserApi->ReqQryCFMMCTradingAccountKey(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryCFMMCTradingAccountKey(CThostFtdcCFMMCTradingAccountKeyField *pCFMMCTradingAccountKey, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pCFMMCTradingAccountKey)
	{
		//memcpy(&g_Cfmmc,pCFMMCTradingAccountKey,sizeof(CThostFtdcCFMMCTradingAccountKeyField));

		char strMsg[1000];
		sprintf(strMsg,CFMMC_TMPL,pCFMMCTradingAccountKey->ParticipantID,pCFMMCTradingAccountKey->AccountID,
			pCFMMCTradingAccountKey->KeyID,pCFMMCTradingAccountKey->CurrentKey);
		ShellExecuteA(NULL,"open",strMsg,NULL, NULL, SW_SHOW);

	}
	//if(bIsLast) SetEvent(g_hEvent);	
}

void CtpTraderSpi::OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus)
{
	if(pInstrumentStatus)
	{
		
		
	}
}

void CtpTraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	IsErrorRspInfo(pRspInfo);
}

bool CtpTraderSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// ���ErrorID != 0, ˵���յ��˴������Ӧ
	bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	return ret;
}

void CtpTraderSpi::ClrAllVecs()
{
	m_orderVec.clear();
	m_tradeVec.clear();
	m_InsinfVec.clear();
	m_MargRateVec.clear();
	m_StmiVec.clear();
	m_AccRegVec.clear();
	m_TdCodeVec.clear();
	m_InvPosVec.clear();
	m_BfTransVec.clear();
}