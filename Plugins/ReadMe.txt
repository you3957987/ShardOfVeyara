0 - 1. uproject 파일 있는 폴더에 Plugins라는 폴더 안에다가 넣기. 없으면 폴더 생성
0 - 2. 프로젝트 폴더의 Binaries, Intermediate , Saved 외의 플러그인 폴더 내의  Binaries, Intermediate 이거 삭제하고 sln( 에디터마다 다를 수도? ) 삭제후
        uproject 우클릭해서 generate visual studio project file 하고 빌드 하고 실행하는거 추천.

1. 액터 배치 패널에서 NavMeshVolume 추가. 안하면 ai가 플레이어 추적을 못함
1 - 1. navmesh 넣고 나서 자동으로 생성 되는 Recast ~ 위치는 0, 0, 0 으로 해야함. 이거 z축으로 100 올리면 nav 추적을 100 위로 올려서 이상해짐.
1 - 2. Recast 에서 에이전트 반경은 넉넉히 50 정도로 하기. + NavMesh를 그냥 넓히면 맵 중간 중간에 이동 가능 영역 생기는거 고려 해야함!
2. 사용하는 캐릭터 Blueprint에서 태그 추가 ->Player 추가.  안하면 적들 공격을 플레이어가 감지 못함
3. 적들한테는 Enemy 태그 추가하기도 나중에 필요