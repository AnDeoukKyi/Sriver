package company.co.kr.sriverforuser;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Vector;

public class Dijkstra {
    int INF = 987654321;
    int[][] LEN;
    int[] dist;
    boolean[] visited;
    int[] prev;
    int N;
    Vector<Integer> steps = new Vector<Integer>();

    //호출순서
    //INIT->CONNECTNODE or DONNECT->SEARCHPATH

    public void INIT(int n){
        this.N = n;
        LEN = new int[N][N];
        dist = new int[N];
        visited = new boolean[N];
        prev = new int[N];
        //INF로 채움
        for(int i=0; i<N; i++)
            Arrays.fill(LEN[i], INF);
        // start node부터 i node까지의 최단거리
        Arrays.fill(dist, INF);
        // 각 노드별로 방문한 적이 있는지 표시
        Arrays.fill(visited, false);
        // 각노드의 이전 노드번호
        Arrays.fill(prev, 0);
        steps.clear();
    }

    public void CONNECT(int a1, int a2, int len){
        if (len == 0) len = 10;
        LEN[a1][a2] = LEN[a2][a1] = len;
    }

    public void DONNECT(int a1, int a2){
        LEN[a1][a2] = LEN[a2][a1] = INF;
    }

    public int shortestPath(int start, int end){
        // 기저
        if(start == end){
            return dist[end];
        }
        // 방문한 노드들을 차례대로 담는다
        visited[start] = true;

        // 연결된 노드들 중 짧은 거리 노드의 인덱스
        int min=INF;

        for(int i=0; i<N; i++){

            // start node의 하위 트리 탐색
            // 조건1 : 방문이력x
            // 조건2 : start node와 연결됨
            if(LEN[start][i]!=INF && visited[i]==false){

                // 기존값과 start node를 경유해서 오는 거리 비교 후 최소거리로 업데이트
                if(dist[i] > dist[start] + LEN[start][i]){
                    dist[i] = dist[start] + LEN[start][i];
                    prev[i] = start;    // 최소경로를 제공하는 전노드
                }
                // 다음 경로 node 갱신
                // 조건 : 최소거리값이 작은 노드
                if(min==INF || dist[min] > dist[i]){
                    min = i;
                }
            }
        }
        // 최소경로를 시작으로 다시 recursive 하게 방문한다.
        shortestPath(min, end);
        return 0;
    }
    public void makingPath(int start, int end, int node){
        if(node == start){
            steps.add(node);
            return;
        }
        makingPath(start, end, prev[node]);
        steps.add(node);
    }


    public ArrayList<Integer> SEARCHPATH(){
        ArrayList<Integer> path = new ArrayList<>();
        dist[0] = 0;
        shortestPath(0,N-1);
        makingPath(0,N-1,N-1);
        for(int node : steps){
            path.add(node);
        }
        return path;
    }
}
