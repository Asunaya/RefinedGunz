#pragma once

struct TriangleConnectivityInfo
{
	int NeighborTriIndex[3];	// 이웃 triangle index
	TriangleConnectivityInfo()
	{
		NeighborTriIndex[0] = -1;
		NeighborTriIndex[1] = -1;
		NeighborTriIndex[2] = -1;
	}
};

// PreProcessing Please...
TriangleConnectivityInfo* MakeTriangleInfoList( RMeshNode* pMeshNode )
{
	int pv1, nv1, pv2, nv2;
	
	// 모든 페이스가 삼각형임을 가정한다.. 아님 낭패
	int n = pMeshNode->m_face_num;
	
	TriangleConnectivityInfo* pTriConnectInfoList	= new TriangleConnectivityInfo[n];

	for( int i = 0; i < n; ++i )
	{
		for( int j = 0; j < n; ++j )
		{
			if( i == j ) continue;			// 같은 삼각형 끼리는 비교 안함
			for( int k = 0; k < 3; ++k )
			{
				pv1 = ( l == 0 ? 2 : l -1 );
				nv1 = ( l == 2 ? 0 : l +1 );
				if( pTriConnectInfoList[k] != -1 ) continue;	// 이미 이웃하고 있는 삼각형이 있다
				for( int l = 0; l < 3; ++l )
				{
					pv2 = ( l == 0 ? 2 : l -1 );
					nv2 = ( l == 2 ? 0 : l +1 );
					if( pMeshNode->m_face_list[i].m_point_index[k] == pMeshNode->m_face_list[j].m_point_index[l] && pMeshNode->m_face_list[i].m_point_index[nv1] == pMeshNode->m_face_list[j].m_point_index[nv2])
					{
						pTriConnectInfoList[i].NeighborTriIndex[k] = j;	// i의 k번째 이웃은 j
						pTriConnectInfoList[j].NeighborTriIndex[l] = i;		// j의 l번째 이웃은 i
					}
					if( pMeshNode->m_face_list[i].m_point_index[k] == pMeshNode->m_face_list[j].m_point_index[pv2] && pMeshNode->m_face_list[i].m_point_index[nv1] == pMeshNode->m_face_list[j].m_point_index[l] )
					{
						pTriConnectInfoList[i].NeighborTriIndex[k] = j;			// i의 k번째 이웃은 j
						pTriConnectInfoList[j].NeighborTriIndex[pv2] = i;		// j의 pv2번째 이웃은 i
					}
				}
			}
		}
	}

	return pTriConnectInfoList;
}