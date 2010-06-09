/* ====================================================================================================================

  The copyright in this software is being made available under the License included below.
  This software may be subject to other third party and   contributor rights, including patent rights, and no such
  rights are granted under this license.

  Copyright (c) 2010, SAMSUNG ELECTRONICS CO., LTD. and BRITISH BROADCASTING CORPORATION
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted only for
  the purpose of developing standards within the Joint Collaborative Team on Video Coding and for testing and
  promoting such standards. The following conditions are required to be met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
      the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
      the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of SAMSUNG ELECTRONICS CO., LTD. nor the name of the BRITISH BROADCASTING CORPORATION
      may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 * ====================================================================================================================
*/

/** \file     TEncEntropy.cpp
    \brief    entropy encoder class
*/

#include "TEncEntropy.h"

Void TEncEntropy::setEntropyCoder ( TEncEntropyIf* e, TComSlice* pcSlice )
{
  m_pcEntropyCoderIf = e;
  m_pcEntropyCoderIf->setSlice ( pcSlice );
}

Void TEncEntropy::encodeSliceHeader ( TComSlice* pcSlice )
{
  m_pcEntropyCoderIf->codeSliceHeader( pcSlice );
  return;
}

Void TEncEntropy::encodeTerminatingBit      ( UInt uiIsLast )
{
  m_pcEntropyCoderIf->codeTerminatingBit( uiIsLast );

  return;
}

Void TEncEntropy::encodeSliceFinish()
{
  m_pcEntropyCoderIf->codeSliceFinish();
}

Void TEncEntropy::encodePPS( TComPPS* pcPPS )
{
  m_pcEntropyCoderIf->codePPS( pcPPS );
  return;
}

Void TEncEntropy::encodeSPS( TComSPS* pcSPS )
{
  m_pcEntropyCoderIf->codeSPS( pcSPS );
  return;
}

Void TEncEntropy::encodeSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
#if HHI_MRG
  if ( pcCU->getSlice()->getSPS()->getUseMRG() )
  {
    return;
  }
#endif

  if( bRD )
    uiAbsPartIdx = 0;

  if ( pcCU->getSlice()->isIntra() )
  {
    return;
  }

  m_pcEntropyCoderIf->codeSkipFlag( pcCU, uiAbsPartIdx );
}

#if HHI_MRG
Void TEncEntropy::encodeMergeFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  m_pcEntropyCoderIf->codeMergeFlag( pcCU, uiAbsPartIdx );
}
  
Void TEncEntropy::encodeMergeIndex( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  m_pcEntropyCoderIf->codeMergeIndex( pcCU, uiAbsPartIdx );
}
#endif

Void TEncEntropy::encodeAlfParam(ALFParam* pAlfParam)
{
#if HHI_ALF
  m_pcEntropyCoderIf->codeAlfFlag(pAlfParam->alf_flag);
  if (!pAlfParam->alf_flag)
    return;
  Int pos;
  Int iCenterPos ;

  // filter parameters for luma
  // horizontal filter
  AlfFilter *pHorizontalFilter = &(pAlfParam->acHorizontalAlfFilter[0]) ;
  iCenterPos = ( pHorizontalFilter->iFilterSymmetry == 0 ) ? (pHorizontalFilter->iFilterLength + 1) >> 1 : pHorizontalFilter->iNumOfCoeffs - 1 ;

  m_pcEntropyCoderIf->codeAlfUvlc( (pHorizontalFilter->iFilterLength-ALF_MIN_LENGTH)/2 );
  m_pcEntropyCoderIf->codeAlfUvlc( pHorizontalFilter->iFilterSymmetry );

  Int iCoeff;
  for(pos=0; pos < pHorizontalFilter->iNumOfCoeffs; pos++)
  {
    iCoeff = pHorizontalFilter->aiQuantFilterCoeffs[pos] ;
    m_pcEntropyCoderIf->codeAlfCoeff(iCoeff,pHorizontalFilter->iFilterLength, pos );
  }
#if ALF_DC_CONSIDERED
  m_pcEntropyCoderIf->codeAlfDc( pHorizontalFilter->aiQuantFilterCoeffs[ pHorizontalFilter->iNumOfCoeffs ] );
#endif
  // vertical filter
  AlfFilter *pVerticalFilter = &(pAlfParam->acVerticalAlfFilter[0]) ;

  m_pcEntropyCoderIf->codeAlfUvlc( (pVerticalFilter->iFilterLength-ALF_MIN_LENGTH)/2 );
  m_pcEntropyCoderIf->codeAlfUvlc( pVerticalFilter->iFilterSymmetry );

  iCenterPos = ( pVerticalFilter->iFilterSymmetry == 0 ) ? (pVerticalFilter->iFilterLength + 1) >> 1 : pVerticalFilter->iNumOfCoeffs - 1 ;
  for(pos=0; pos < pVerticalFilter->iNumOfCoeffs ; pos++ )
  {
    iCoeff = pVerticalFilter->aiQuantFilterCoeffs[pos] ;
    m_pcEntropyCoderIf->codeAlfCoeff(iCoeff,pVerticalFilter->iFilterLength, pos );
  }
#if ALF_DC_CONSIDERED
  m_pcEntropyCoderIf->codeAlfDc( pVerticalFilter->aiQuantFilterCoeffs[ pVerticalFilter->iNumOfCoeffs ] );
#endif
  // filter parameters for chroma
  m_pcEntropyCoderIf->codeAlfUvlc(pAlfParam->chroma_idc);
  for(Int iPlane = 1; iPlane <3; iPlane++)
  {
    if(pAlfParam->chroma_idc&iPlane)
    {
      m_pcEntropyCoderIf->codeAlfUvlc( pAlfParam->aiPlaneFilterMapping[iPlane] ) ;
      if( pAlfParam->aiPlaneFilterMapping[iPlane] == iPlane )
      {
        // horizontal filter
        pHorizontalFilter = &(pAlfParam->acHorizontalAlfFilter[iPlane]) ;
        iCenterPos = ( pHorizontalFilter->iFilterSymmetry == 0 ) ? (pHorizontalFilter->iFilterLength + 1) >> 1 : pHorizontalFilter->iNumOfCoeffs - 1 ;

        m_pcEntropyCoderIf->codeAlfUvlc( (pHorizontalFilter->iFilterLength-ALF_MIN_LENGTH)/2 );
        m_pcEntropyCoderIf->codeAlfUvlc( pHorizontalFilter->iFilterSymmetry );

        for(pos=0; pos < pHorizontalFilter->iNumOfCoeffs; pos++)
        {
          iCoeff = pHorizontalFilter->aiQuantFilterCoeffs[pos] ;
          m_pcEntropyCoderIf->codeAlfCoeff(iCoeff,pHorizontalFilter->iFilterLength, pos );
        }
#if ALF_DC_CONSIDERED
        m_pcEntropyCoderIf->codeAlfDc( pHorizontalFilter->aiQuantFilterCoeffs[ pHorizontalFilter->iNumOfCoeffs ] );
#endif
        // vertical filter
        pVerticalFilter = &(pAlfParam->acVerticalAlfFilter[iPlane]) ;

        m_pcEntropyCoderIf->codeAlfUvlc( (pVerticalFilter->iFilterLength-ALF_MIN_LENGTH)/2 );
        m_pcEntropyCoderIf->codeAlfUvlc( pVerticalFilter->iFilterSymmetry );

        iCenterPos = ( pVerticalFilter->iFilterSymmetry == 0 ) ? (pVerticalFilter->iFilterLength + 1) >> 1 : pVerticalFilter->iNumOfCoeffs - 1 ;
        for(pos=0; pos < pVerticalFilter->iNumOfCoeffs ; pos++ )
        {
          iCoeff = pVerticalFilter->aiQuantFilterCoeffs[pos] ;
    			m_pcEntropyCoderIf->codeAlfCoeff(iCoeff,pVerticalFilter->iFilterLength, pos );
        }
#if ALF_DC_CONSIDERED
        m_pcEntropyCoderIf->codeAlfDc( pVerticalFilter->aiQuantFilterCoeffs[ pVerticalFilter->iNumOfCoeffs ] );
#endif
      }
    }
  }

  // region control parameters for luma
  m_pcEntropyCoderIf->codeAlfFlag(pAlfParam->cu_control_flag);
  if (pAlfParam->cu_control_flag)
  {
    assert( (pAlfParam->cu_control_flag && m_pcEntropyCoderIf->getAlfCtrl()) || (!pAlfParam->cu_control_flag && !m_pcEntropyCoderIf->getAlfCtrl()));
    m_pcEntropyCoderIf->codeAlfFlag( pAlfParam->bSeparateQt );
    m_pcEntropyCoderIf->codeAlfCtrlDepth();
  }
#else
  m_pcEntropyCoderIf->codeAlfFlag(pAlfParam->alf_flag);
  if (!pAlfParam->alf_flag)
    return;
  Int pos;
  // filter parameters for luma
  m_pcEntropyCoderIf->codeAlfUvlc((pAlfParam->tap-5)/2);
  for(pos=0; pos<pAlfParam->num_coeff; pos++)
  {
    m_pcEntropyCoderIf->codeAlfSvlc(pAlfParam->coeff[pos]);
  }

  // filter parameters for chroma
  m_pcEntropyCoderIf->codeAlfUvlc(pAlfParam->chroma_idc);
  if(pAlfParam->chroma_idc)
  {
    m_pcEntropyCoderIf->codeAlfUvlc((pAlfParam->tap_chroma-5)/2);

    // filter coefficients for chroma
    for(pos=0; pos<pAlfParam->num_coeff_chroma; pos++)
    {
      m_pcEntropyCoderIf->codeAlfSvlc(pAlfParam->coeff_chroma[pos]);
    }
  }

  // region control parameters for luma
  m_pcEntropyCoderIf->codeAlfFlag(pAlfParam->cu_control_flag);
  if (pAlfParam->cu_control_flag)
  {
    assert( (pAlfParam->cu_control_flag && m_pcEntropyCoderIf->getAlfCtrl()) || (!pAlfParam->cu_control_flag && !m_pcEntropyCoderIf->getAlfCtrl()));
    m_pcEntropyCoderIf->codeAlfCtrlDepth();
  }
#endif
}

#if HHI_ALF
Void TEncEntropy::encodeAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD, Bool bSeparateQt )
{
  if( bRD )
    uiAbsPartIdx = 0;

  if( bSeparateQt )
  {
    m_pcEntropyCoderIf->codeAlfQTCtrlFlag( pcCU, uiAbsPartIdx );
  }
  else
  {
    m_pcEntropyCoderIf->codeAlfCtrlFlag( pcCU, uiAbsPartIdx );
  }
}

Void TEncEntropy::encodeAlfQTSplitFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiMaxDepth, Bool bRD )
{
  if( bRD )
      uiAbsPartIdx = 0;

  m_pcEntropyCoderIf->codeAlfQTSplitFlag( pcCU, uiAbsPartIdx, uiDepth, uiMaxDepth );
}
#else
Void TEncEntropy::encodeAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  m_pcEntropyCoderIf->codeAlfCtrlFlag( pcCU, uiAbsPartIdx );
}
#endif

Void TEncEntropy::encodePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  if ( pcCU->getSlice()->isIntra() )
  {
    return;
  }

#if HHI_MRG
  if ( pcCU->getMergeFlag( uiAbsPartIdx ) )
  {
    return;
  }
#endif

  if (pcCU->isSkipped( uiAbsPartIdx ))
    return;

  m_pcEntropyCoderIf->codePredMode( pcCU, uiAbsPartIdx );
}

// Split mode
Void TEncEntropy::encodeSplitFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  m_pcEntropyCoderIf->codeSplitFlag( pcCU, uiAbsPartIdx, uiDepth );
}

Void TEncEntropy::encodePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

#if HHI_MRG
  if ( pcCU->getMergeFlag( uiAbsPartIdx ) )
  {
    return;
  }
#endif

  if ( pcCU->isSkip( uiAbsPartIdx ) )
    return;

  m_pcEntropyCoderIf->codePartSize( pcCU, uiAbsPartIdx, uiDepth );
}

#if HHI_RQT
Void TEncEntropy::xEncodeTransformSubdiv( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiInnerQuadIdx )
{
  const UInt uiSubdiv = pcCU->getTransformIdx( uiAbsPartIdx ) + pcCU->getDepth( uiAbsPartIdx ) > uiDepth;
  const UInt uiLog2TrafoSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth()]+2 - uiDepth;

#if HHI_RQT_INTRA
  if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA && pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_NxN && uiDepth == pcCU->getDepth(uiAbsPartIdx) )
  {
    assert( uiSubdiv );
  }
  else
#endif
  if( uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
  {
    assert( uiSubdiv );
  }
  else if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    assert( !uiSubdiv );
  }
  else
  {
    assert( uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() );
    m_pcEntropyCoderIf->codeTransformSubdivFlag( uiSubdiv, uiDepth );
  }

#if HHI_RQT_CHROMA_CBF_MOD
  if( pcCU->getPredictionMode(uiAbsPartIdx) != MODE_INTRA && uiLog2TrafoSize <= pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
  {
    const UInt uiTrDepthCurr = uiDepth - pcCU->getDepth( uiAbsPartIdx );
    const Bool bFirstCbfOfCU = uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() || uiTrDepthCurr == 0;
    if( bFirstCbfOfCU || uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
    {
      if( bFirstCbfOfCU || pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepthCurr - 1 ) )
      {
        m_pcEntropyCoderIf->codeQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepthCurr );
      }
      if( bFirstCbfOfCU || pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepthCurr - 1 ) )
      {
        m_pcEntropyCoderIf->codeQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepthCurr );
      }
    }
    else if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
    {
      assert( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepthCurr ) == pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepthCurr - 1 ) );
      assert( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepthCurr ) == pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepthCurr - 1 ) );
    }
  }
#endif

  if( uiSubdiv )
  {
    ++uiDepth;
    const UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1);
    xEncodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, 0 );
    uiAbsPartIdx += uiQPartNum;
    xEncodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, 1 );
    uiAbsPartIdx += uiQPartNum;
    xEncodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, 2 );
    uiAbsPartIdx += uiQPartNum;
    xEncodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, 3 );
  }
  else
  {
    {
      DTRACE_CABAC_V( g_nSymbolCounter++ );
      DTRACE_CABAC_T( "\tTrIdx: abspart=" );
      DTRACE_CABAC_V( uiAbsPartIdx );
      DTRACE_CABAC_T( "\tdepth=" );
      DTRACE_CABAC_V( uiDepth );
      DTRACE_CABAC_T( "\ttrdepth=" );
      DTRACE_CABAC_V( pcCU->getTransformIdx( uiAbsPartIdx ) );
      DTRACE_CABAC_T( "\n" );
    }
    UInt uiLumaTrMode, uiChromaTrMode;
    pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx( uiAbsPartIdx ), uiLumaTrMode, uiChromaTrMode );
    m_pcEntropyCoderIf->codeQtCbf( pcCU, uiAbsPartIdx, TEXT_LUMA, uiLumaTrMode );
#if HHI_RQT_CHROMA_CBF_MOD
    if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA )
#endif
    {
      Bool bCodeChroma = true;
      if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
      {
        UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
        bCodeChroma  = ( ( uiAbsPartIdx % uiQPDiv ) == 0 );
      }
      if( bCodeChroma )
      {
        m_pcEntropyCoderIf->codeQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, uiChromaTrMode );
        m_pcEntropyCoderIf->codeQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, uiChromaTrMode );
      }
    }
  }
}
#endif

// transform index
Void TEncEntropy::encodeTransformIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD )
{
#if HHI_RQT
  assert( !bRD ); // parameter bRD can be removed
#endif
  if( bRD )
    uiAbsPartIdx = 0;

#if HHI_RQT
  if( pcCU->getSlice()->getSPS()->getQuadtreeTUFlag() )
  {
    DTRACE_CABAC_V( g_nSymbolCounter++ )
    DTRACE_CABAC_T( "\tdecodeTransformIdx()\tCUDepth=" )
    DTRACE_CABAC_V( uiDepth )
    DTRACE_CABAC_T( "\n" )
    xEncodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, 0 );
  }
  else
#endif
  m_pcEntropyCoderIf->codeTransformIdx( pcCU, uiAbsPartIdx, uiDepth );
}

// ROT index
Void TEncEntropy::encodeROTindex  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

    if (pcCU->getPredictionMode( uiAbsPartIdx )==MODE_INTRA)
    {
      if( ( pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA) + pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U) + pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V) ) == 0 )
      {
        return;
      }
      m_pcEntropyCoderIf->codeROTindex( pcCU, uiAbsPartIdx, bRD );
  }
}

// CIP index
Void TEncEntropy::encodeCIPflag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD )
{
  if( bRD )
  uiAbsPartIdx = 0;

  if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
    m_pcEntropyCoderIf->codeCIPflag( pcCU, uiAbsPartIdx, bRD );
  }
}

// Intra direction for Luma
Void TEncEntropy::encodeIntraDirModeLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  m_pcEntropyCoderIf->codeIntraDirLumaAdi( pcCU, uiAbsPartIdx );
}

#if HHI_AIS
// BB: Intra ref. samples filtering for Luma
Void TEncEntropy::encodeIntraFiltFlagLuma ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  // DC (mode 2) always uses DEFAULT_IS so no signaling needed
  // (no g_aucIntraModeOrder[][] mapping needed because mode 2 always mapped to 2)
  if( (pcCU->getSlice()->getSPS()->getUseAIS()) && (pcCU->getLumaIntraDir( uiAbsPartIdx ) != 2) )
    m_pcEntropyCoderIf->codeIntraFiltFlagLumaAdi( pcCU, uiAbsPartIdx );
}
#endif

// Intra direction for Chroma
Void TEncEntropy::encodeIntraDirModeChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  m_pcEntropyCoderIf->codeIntraDirChroma( pcCU, uiAbsPartIdx );
}

Void TEncEntropy::encodePredInfo( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

#if HHI_MRG
  if ( pcCU->getMergeFlag( uiAbsPartIdx ) )
  {
    return;
  }
#endif

  if (pcCU->isSkip( uiAbsPartIdx ))
  {
    if (pcCU->getSlice()->isInterB())
    {
      encodeInterDir(pcCU, uiAbsPartIdx, bRD);
    }
    if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 ) //if ( ref. frame list0 has at least 1 entry )
    {
      encodeMVPIdx( pcCU, uiAbsPartIdx, REF_PIC_LIST_0);
    }
    if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) //if ( ref. frame list1 has at least 1 entry )
    {
      encodeMVPIdx( pcCU, uiAbsPartIdx, REF_PIC_LIST_1);
    }
    return;
  }

  PartSize eSize = pcCU->getPartitionSize( uiAbsPartIdx );

  if( pcCU->isIntra( uiAbsPartIdx ) )                                 // If it is Intra mode, encode intra prediction mode.
  {
    if( eSize == SIZE_NxN )                                         // if it is NxN size, encode 4 intra directions.
    {
      UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( pcCU->getDepth(uiAbsPartIdx) << 1 ) ) >> 2;
      // if it is NxN size, this size might be the smallest partition size.
      encodeIntraDirModeLuma( pcCU, uiAbsPartIdx                  );
      encodeIntraDirModeLuma( pcCU, uiAbsPartIdx + uiPartOffset   );
      encodeIntraDirModeLuma( pcCU, uiAbsPartIdx + uiPartOffset*2 );
      encodeIntraDirModeLuma( pcCU, uiAbsPartIdx + uiPartOffset*3 );
#if HHI_AIS
      //BB: intra ref. samples filtering flag
      encodeIntraFiltFlagLuma( pcCU, uiAbsPartIdx                  );
      encodeIntraFiltFlagLuma( pcCU, uiAbsPartIdx + uiPartOffset   );
      encodeIntraFiltFlagLuma( pcCU, uiAbsPartIdx + uiPartOffset*2 );
      encodeIntraFiltFlagLuma( pcCU, uiAbsPartIdx + uiPartOffset*3 );
      //
#endif
      encodeIntraDirModeChroma( pcCU, uiAbsPartIdx, bRD );
    }
    else                                                              // if it is not NxN size, encode 1 intra directions
    {
      encodeIntraDirModeLuma  ( pcCU, uiAbsPartIdx );
#if HHI_AIS
      //BB: intra ref. samples filtering flag
      encodeIntraFiltFlagLuma ( pcCU, uiAbsPartIdx );
      //
#endif
      encodeIntraDirModeChroma( pcCU, uiAbsPartIdx, bRD );
    }
  }
  else                                                                // if it is Inter mode, encode motion vector and reference index
  {
    encodeInterDir( pcCU, uiAbsPartIdx, bRD );

    if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )       // if ( ref. frame list0 has at least 1 entry )
    {
      encodeRefFrmIdx ( pcCU, uiAbsPartIdx, REF_PIC_LIST_0, bRD );
      encodeMvd       ( pcCU, uiAbsPartIdx, REF_PIC_LIST_0, bRD );
      encodeMVPIdx    ( pcCU, uiAbsPartIdx, REF_PIC_LIST_0      );
    }

    if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )       // if ( ref. frame list1 has at least 1 entry )
    {
      encodeRefFrmIdx ( pcCU, uiAbsPartIdx, REF_PIC_LIST_1, bRD );
      encodeMvd       ( pcCU, uiAbsPartIdx, REF_PIC_LIST_1, bRD );
      encodeMVPIdx    ( pcCU, uiAbsPartIdx, REF_PIC_LIST_1      );
    }
  }
}

#if HHI_MRG
Void TEncEntropy::encodeMergeInfo( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if ( !pcCU->getSlice()->getSPS()->getUseMRG() )
  {
    return;
  }

  if ( pcCU->getSlice()->isIntra() )
  {
    return;
  }

  // find left and top vectors. take vectors from PUs to the left and above.
  TComMvField cMvFieldNeighbours[4]; // above ref_list_0, above ref_list_1, left ref_list_0, left ref_list_1
  UInt uiNeighbourInfo;
  UChar uhInterDirNeighbours[2];
  pcCU->getInterMergeCandidates( uiAbsPartIdx, cMvFieldNeighbours, uhInterDirNeighbours, uiNeighbourInfo );
  
  if ( uiNeighbourInfo )
  {
    // at least one merge candidate exists
    encodeMergeFlag( pcCU, uiAbsPartIdx, bRD );
  }
  else
  {
    assert( !pcCU->getMergeFlag( uiAbsPartIdx ) );
  }

  if ( !pcCU->getMergeFlag( uiAbsPartIdx ) )
  {
    // CU is not merged
    return;
  }

  if ( uiNeighbourInfo == 3 )
  {
    // different merge candidates exist. write Merge Index
    encodeMergeIndex( pcCU, uiAbsPartIdx, bRD );
  }
  
}
#endif

Void TEncEntropy::encodeInterDir( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  assert( !pcCU->isIntra( uiAbsPartIdx ) );
  assert( !pcCU->isSkip( uiAbsPartIdx ) || pcCU->getSlice()->isInterB());

  if( bRD )
    uiAbsPartIdx = 0;

  if ( !pcCU->getSlice()->isInterB() )
  {
    return;
  }

  UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( pcCU->getDepth(uiAbsPartIdx) << 1 ) ) >> 2;

  switch ( pcCU->getPartitionSize( uiAbsPartIdx ) )
  {
  case SIZE_2Nx2N:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      break;
    }

  case SIZE_2NxN:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      uiAbsPartIdx += uiPartOffset << 1;
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      break;
    }

  case SIZE_Nx2N:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      uiAbsPartIdx += uiPartOffset;
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      break;
    }

  case SIZE_NxN:
    {
      for ( Int iPartIdx = 0; iPartIdx < 4; iPartIdx++ )
      {
        m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
        uiAbsPartIdx += uiPartOffset;
      }
      break;
    }
  case SIZE_2NxnU:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx + (uiPartOffset>>1) );
      break;
    }
  case SIZE_2NxnD:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx + (uiPartOffset<<1) + (uiPartOffset>>1) );
      break;
    }
  case SIZE_nLx2N:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx + (uiPartOffset>>2) );
      break;
    }
  case SIZE_nRx2N:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx + uiPartOffset + (uiPartOffset>>2) );
      break;
    }
  default:
    break;
  }

  return;
}

Void TEncEntropy::encodeMVPIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( pcCU->getDepth(uiAbsPartIdx) << 1 ) ) >> 2;

  switch ( pcCU->getPartitionSize( uiAbsPartIdx ) )
  {
  case SIZE_2Nx2N:
    {
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
      {
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );
      }
      break;
    }

  case SIZE_2NxN:
    {
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
      {
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );
      }
      uiAbsPartIdx += uiPartOffset << 1;
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
      {
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );
      }
      break;
    }

  case SIZE_Nx2N:
    {
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
      {
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );
      }

      uiAbsPartIdx += uiPartOffset;
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
      {
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );
      }

      break;
    }

  case SIZE_NxN:
    {
      for ( Int iPartIdx = 0; iPartIdx < 4; iPartIdx++ )
      {
        if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
        {
          m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );
        }
        uiAbsPartIdx += uiPartOffset;
      }
      break;
    }
  case SIZE_2NxnU:
    {
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += (uiPartOffset>>1);
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  case SIZE_2NxnD:
    {
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += (uiPartOffset<<1) + (uiPartOffset>>1);
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  case SIZE_nLx2N:
    {
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += (uiPartOffset>>2);
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  case SIZE_nRx2N:
    {
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += uiPartOffset + (uiPartOffset>>2);
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  default:
    break;
  }

  return;

}

Void TEncEntropy::encodeRefFrmIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList, Bool bRD )
{
  assert( !pcCU->isIntra( uiAbsPartIdx ) );

  if( bRD )
    uiAbsPartIdx = 0;

  if ( ( pcCU->getSlice()->getNumRefIdx( eRefList ) == 1 ) || pcCU->isSkip( uiAbsPartIdx ) )
  {
    return;
  }

  UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( pcCU->getDepth(uiAbsPartIdx) << 1 ) ) >> 2;

  switch ( pcCU->getPartitionSize( uiAbsPartIdx ) )
  {
  case SIZE_2Nx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );
      }
      break;
    }

  case SIZE_2NxN:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );
      }

      uiAbsPartIdx += uiPartOffset << 1;
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );
      }
      break;
    }

  case SIZE_Nx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );
      }

      uiAbsPartIdx += uiPartOffset;
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );
      }
      break;
    }

  case SIZE_NxN:
    {
      for ( Int iPartIdx = 0; iPartIdx < 4; iPartIdx++ )
      {
        if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        {
          m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );
        }
        uiAbsPartIdx += uiPartOffset;
      }
      break;
    }
  case SIZE_2NxnU:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += (uiPartOffset>>1);

      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  case SIZE_2NxnD:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += (uiPartOffset<<1) + (uiPartOffset>>1);

      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  case SIZE_nLx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += (uiPartOffset>>2);

      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  case SIZE_nRx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += uiPartOffset + (uiPartOffset>>2);

      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  default:
    break;
  }

  return;
}

Void TEncEntropy::encodeMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList, Bool bRD )
{
  assert( !pcCU->isIntra( uiAbsPartIdx ) );

  if( bRD )
    uiAbsPartIdx = 0;

  if ( pcCU->isSkip( uiAbsPartIdx ) )
  {
    return;
  }

  UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( pcCU->getDepth(uiAbsPartIdx) << 1 ) ) >> 2;

  switch ( pcCU->getPartitionSize( uiAbsPartIdx ) )
  {
  case SIZE_2Nx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );
      }
      break;
    }

  case SIZE_2NxN:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );
      }

      uiAbsPartIdx += uiPartOffset << 1;
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );
      }
      break;
    }

  case SIZE_Nx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );
      }

      uiAbsPartIdx += uiPartOffset;
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );
      }
      break;
    }

  case SIZE_NxN:
    {
      for ( Int iPartIdx = 0; iPartIdx < 4; iPartIdx++ )
      {
        if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        {
          m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );
        }
        uiAbsPartIdx += uiPartOffset;
      }
      break;
    }
  case SIZE_2NxnU:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += (uiPartOffset>>1);

      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  case SIZE_2NxnD:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += (uiPartOffset<<1) + (uiPartOffset>>1);

      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  case SIZE_nLx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += (uiPartOffset>>2);

      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  case SIZE_nRx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += uiPartOffset + (uiPartOffset>>2);

      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  default:
    break;
  }

  return;
}

#if HHI_RQT
Void TEncEntropy::encodeQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  m_pcEntropyCoderIf->codeQtCbf( pcCU, uiAbsPartIdx, eType, uiTrDepth );
}

Void TEncEntropy::encodeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx )
{
  m_pcEntropyCoderIf->codeTransformSubdivFlag( uiSymbol, uiCtx );
}
#endif

// Coded block flag
Void TEncEntropy::encodeCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  m_pcEntropyCoderIf->codeCbf( pcCU, uiAbsPartIdx, eType, uiTrDepth );
}

// dQP
Void TEncEntropy::encodeQP( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  if ( pcCU->getSlice()->getSPS()->getUseDQP() )
  {
    m_pcEntropyCoderIf->codeDeltaQP( pcCU, uiAbsPartIdx );
  }
}


// texture
Void TEncEntropy::xEncodeCoeff( TComDataCU* pcCU, TCoeff* pcCoeff, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, UInt uiTrIdx, UInt uiCurrTrIdx, TextType eType, Bool bRD )
{
  if ( pcCU->getCbf( uiAbsPartIdx, eType, uiTrIdx ) )
  {
#if HHI_RQT
    UInt uiLumaTrMode, uiChromaTrMode;
    pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx( uiAbsPartIdx ), uiLumaTrMode, uiChromaTrMode );
    const UInt uiStopTrMode = eType == TEXT_LUMA ? uiLumaTrMode : uiChromaTrMode;

    assert( pcCU->getSlice()->getSPS()->getQuadtreeTUFlag() || uiStopTrMode == uiCurrTrIdx ); // as long as quadtrees are not used for residual transform

    if( uiTrIdx == uiStopTrMode )
#else
    if( uiCurrTrIdx == uiTrIdx )
#endif
    {
#if HHI_RQT
      assert( !bRD ); // parameter bRD can be removed

      UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth ] + 2;
      if( eType != TEXT_LUMA && uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
      {
        UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
        if( ( uiAbsPartIdx % uiQPDiv ) != 0 )
        {
          return;
        }
        uiWidth  <<= 1;
        uiHeight <<= 1;
      }
#endif
      m_pcEntropyCoderIf->codeCoeffNxN( pcCU, pcCoeff, uiAbsPartIdx, uiWidth, uiHeight, uiDepth, eType, bRD );
    }
    else
    {
#if HHI_RQT
      {
        DTRACE_CABAC_V( g_nSymbolCounter++ );
        DTRACE_CABAC_T( "\tgoing down\tdepth=" );
        DTRACE_CABAC_V( uiDepth );
        DTRACE_CABAC_T( "\ttridx=" );
        DTRACE_CABAC_V( uiTrIdx );
        DTRACE_CABAC_T( "\n" );
      }
#endif
      if( uiCurrTrIdx <= uiTrIdx )
#if HHI_RQT
        assert( pcCU->getSlice()->getSPS()->getQuadtreeTUFlag() );
#else
        assert(0);
#endif

      UInt uiSize;
      uiWidth  >>= 1;
      uiHeight >>= 1;
      uiSize = uiWidth*uiHeight;
      uiDepth++;
      uiTrIdx++;

      UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1);
      UInt uiIdx      = uiAbsPartIdx;

      m_pcEntropyCoderIf->codeCbf( pcCU, uiIdx, eType, uiTrIdx );
      xEncodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType, bRD ); pcCoeff += uiSize; uiIdx += uiQPartNum;

      m_pcEntropyCoderIf->codeCbf( pcCU, uiIdx, eType, uiTrIdx );
      xEncodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType, bRD ); pcCoeff += uiSize; uiIdx += uiQPartNum;

      m_pcEntropyCoderIf->codeCbf( pcCU, uiIdx, eType, uiTrIdx );
      xEncodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType, bRD ); pcCoeff += uiSize; uiIdx += uiQPartNum;

      m_pcEntropyCoderIf->codeCbf( pcCU, uiIdx, eType, uiTrIdx );
      xEncodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType, bRD );
#if HHI_RQT
      {
        DTRACE_CABAC_V( g_nSymbolCounter++ );
        DTRACE_CABAC_T( "\tgoing up\n" );
      }
#endif
    }
  }
}

Void TEncEntropy::encodeCoeff( TComDataCU* pcCU, TCoeff* pCoeff, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, UInt uiMaxTrMode, UInt uiTrMode, TextType eType, Bool bRD )
{
  xEncodeCoeff( pcCU, pCoeff, uiAbsPartIdx, uiDepth, uiWidth, uiHeight, uiTrMode, uiMaxTrMode, eType, bRD );
}

Void TEncEntropy::encodeCoeff( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight )
{
  UInt uiMinCoeffSize = pcCU->getPic()->getMinCUWidth()*pcCU->getPic()->getMinCUHeight();
  UInt uiLumaOffset   = uiMinCoeffSize*uiAbsPartIdx;
  UInt uiChromaOffset = uiLumaOffset>>2;

  UInt uiLumaTrMode, uiChromaTrMode;
  pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx(uiAbsPartIdx), uiLumaTrMode, uiChromaTrMode );

  if( pcCU->isIntra(uiAbsPartIdx) )
  {
#if HHI_RQT_INTRA
    if( pcCU->getSlice()->getSPS()->getQuadtreeTUFlag() )
    {
      DTRACE_CABAC_V( g_nSymbolCounter++ )
      DTRACE_CABAC_T( "\tdecodeTransformIdx()\tCUDepth=" )
      DTRACE_CABAC_V( uiDepth )
      DTRACE_CABAC_T( "\n" )
      xEncodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, 0 );
    }
#endif

    m_pcEntropyCoderIf->codeCbf(pcCU, uiAbsPartIdx, TEXT_LUMA, 0);
    xEncodeCoeff( pcCU, pcCU->getCoeffY()  + uiLumaOffset,   uiAbsPartIdx, uiDepth, uiWidth,    uiHeight,    0, uiLumaTrMode,   TEXT_LUMA     );

    m_pcEntropyCoderIf->codeCbf(pcCU, uiAbsPartIdx, TEXT_CHROMA_U, 0);
    xEncodeCoeff( pcCU, pcCU->getCoeffCb() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_U );

    m_pcEntropyCoderIf->codeCbf(pcCU, uiAbsPartIdx, TEXT_CHROMA_V, 0);
    xEncodeCoeff( pcCU, pcCU->getCoeffCr() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_V );
  }
  else
  {
    m_pcEntropyCoderIf->codeCbf( pcCU, uiAbsPartIdx, TEXT_LUMA, 0 );
    m_pcEntropyCoderIf->codeCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, 0 );
    m_pcEntropyCoderIf->codeCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, 0 );

#if HHI_RQT
    if( pcCU->getSlice()->getSPS()->getQuadtreeTUFlag() || pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0) || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0) || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0) )
#else
    if( pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0) || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0) || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0) )
#endif
      encodeTransformIdx( pcCU, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx) );

    xEncodeCoeff( pcCU, pcCU->getCoeffY()  + uiLumaOffset,   uiAbsPartIdx, uiDepth, uiWidth,    uiHeight,    0, uiLumaTrMode,   TEXT_LUMA     );
    xEncodeCoeff( pcCU, pcCU->getCoeffCb() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_U );
    xEncodeCoeff( pcCU, pcCU->getCoeffCr() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_V );
  }
}

Void TEncEntropy::encodeCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoeff, UInt uiAbsPartIdx, UInt uiTrWidth, UInt uiTrHeight, UInt uiDepth, TextType eType, Bool bRD )
{ // This is for Transform unit processing. This may be used at mode selection stage for Inter.
  m_pcEntropyCoderIf->codeCoeffNxN( pcCU, pcCoeff, uiAbsPartIdx, uiTrWidth, uiTrHeight, uiDepth, eType, bRD );
}


Void TEncEntropy::estimateBit (estBitsSbacStruct* pcEstBitsSbac, UInt uiWidth, TextType eTType)
{
  UInt uiCTXIdx;

  switch(uiWidth)
  {
  case  2: uiCTXIdx = 6; break;
  case  4: uiCTXIdx = 5; break;
  case  8: uiCTXIdx = 4; break;
  case 16: uiCTXIdx = 3; break;
  case 32: uiCTXIdx = 2; break;
  case 64: uiCTXIdx = 1; break;
  default: uiCTXIdx = 0; break;
  }

  eTType = eTType == TEXT_LUMA ? TEXT_LUMA : TEXT_CHROMA;

  m_pcEntropyCoderIf->estBit ( pcEstBitsSbac, uiCTXIdx, eTType );
}

