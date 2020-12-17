#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "File.h"
#include "Ngram.h"
#include "Vocab.h"

using namespace std;

double max(double *arr, int arr_size)
{
    double max_value;
    max_value = arr[0];
    for(int i = 1; i < arr_size; i++)
    {
        if(arr[i] > max_value)
        {
            max_value = arr[i];
        }
    }
    return max_value;
}
int argmax(double *arr, int arr_size)
{
    int max_arg;
    double max_value;
    max_arg = 0;
    max_value = arr[0];
    for(int i = 1; i < arr_size; i++)
    {
        if(arr[i] > max_value)
        {
            max_value = arr[i];
            max_arg = i;

        }
    }
    return max_arg;
}

int main(int argc, char *argv[])
{
    if(argc != 5)
    {
        printf("The command should be ./mydisambig $1 $2 $3 $4\n");
        return 0;
    }
    
    ifstream file_zhuyin(argv[2]);
    vector<string> lines;
    string line;
    while(getline(file_zhuyin, line))
    {
        lines.push_back(line);
    }
    file_zhuyin.close();
    for(int i = 0; i < lines.size(); i++)
    {
        string::iterator itor;
        itor = remove(lines[i].begin(), lines[i].end(), ' ');
        lines[i].erase(itor, lines[i].end());
    }
    int zhu_num;
    zhu_num = 0;
    for(int i = 0; i < lines.size(); i++)
    {
        if(((lines[i][0]&0xFF) == 0xA3) & ((lines[i][1]&0xFF) >= 0x74) & ((lines[i][1]&0xFF) <= 0xBA))
        {
            zhu_num++;
        }
    }
    int *ch_num;
    ch_num = new int[zhu_num];
    char *zhuyin;
    zhuyin = new char[zhu_num*2];
    int zhu_index;
    zhu_index = 0;
    for(int i = 0; i < lines.size(); i++)
    {
        if(((lines[i][0]&0xFF) == 0xA3) & ((lines[i][1]&0xFF) >= 0x74) & ((lines[i][1]&0xFF) <= 0xBA))
            {
                zhuyin[2*zhu_index] = lines[i][0];
                zhuyin[2*zhu_index+1] = lines[i][1];
                ch_num[zhu_index] = 0;
                for(int j = 2; j < lines[i].size(); j = j + 2)
                {
                    ch_num[zhu_index]++;
                }
                zhu_index++;
            }
    }
    char **ch;
    ch = new char*[zhu_num];
    for(int i = 0; i < zhu_num; i++)
    {
        ch[i] = new char[2*ch_num[i]];
    }
    zhu_index = 0;
    for(int i = 0; i < lines.size(); i++)
    {
        if(((lines[i][0]&0xFF) == 0xA3) & ((lines[i][1]&0xFF) >= 0x74) & ((lines[i][1]&0xFF) <= 0xBA))
            {
            
                for(int j = 2; j < lines[i].size(); j = j + 2)
                {
                    ch[zhu_index][j-2] = lines[i][j];
                    ch[zhu_index][j-1] = lines[i][j+1];
                }
                zhu_index++;
            }
    }

    ifstream file_seg(argv[1]);
    vector<string> buffer_seg;
    while(getline(file_seg, line))
    {
        buffer_seg.push_back(line);
    }
    file_seg.close();
    for(int i = 0; i < buffer_seg.size(); i++)
    {
        string::iterator itor;
        itor = remove(buffer_seg[i].begin(), buffer_seg[i].end(), ' ');
        buffer_seg[i].erase(itor, buffer_seg[i].end());
    }
    int ngram_order = 2;
    Vocab voc_2;
    Ngram lm_2(voc_2, ngram_order);
    VocabIndex wid_1, wid_2;
    File file_lm2(argv[3], "r");
    lm_2.read(file_lm2);
    file_lm2.close();
    ofstream file_out(argv[4]);
    vector<string> buffer_out;
    for(int n = 0; n < buffer_seg.size(); n++)
    {
        double **delta;
        int ** phi;
        bool ** oov;
        int * state_num;
        int * frame_zhuindex;
        int frame;
        int * q;
        frame = (buffer_seg[n].size())/2 + 2;
        delta = new double*[frame];
        phi = new int*[frame];
        oov = new bool*[frame];
        state_num = new int[frame];
        frame_zhuindex = new int[frame];
        delta[0] = new double[1];
        delta[frame-1] = new double[1];
        phi[0] = new int[1];
        phi[frame-1] = new int[1];
        oov[0] = new bool[1];
        oov[frame-1] = new bool[1];
        q = new int[frame];
        state_num[0] = 1;
        state_num[frame-1] = 1;
        frame_zhuindex[0] = -2;
        frame_zhuindex[frame-1] = -2;
        q[0] = 0;
        q[frame-1] = 0;
        string ans_str = "";
        
        ans_str.resize(buffer_seg[n].size());
        for(int t = 1; t < frame-1; t++)
        {
            if(((buffer_seg[n][2*(t-1)]&0xFF) != 0xA3) | ((buffer_seg[n][2*(t-1)+1]&0xFF) < 0x74) | ((buffer_seg[n][2*(t-1)+1]&0xFF) > 0xBA))
            {
                delta[t] = new double[1];
                phi[t] = new int[1];
                oov[t] = new bool[1];
                state_num[t] = 1;
                frame_zhuindex[t] = -1; 
            }
            else
            {
                for(int zhu = 0; zhu < zhu_num; zhu++)
                {
                    if((buffer_seg[n][2*(t-1)] == zhuyin[2*zhu]) & (buffer_seg[n][2*(t-1)+1] == zhuyin[2*zhu+1]))
                    {
                        delta[t] = new double[ch_num[zhu]];
                        phi[t] = new int[ch_num[zhu]];
                        oov[t] = new bool[ch_num[zhu]];
                        state_num[t] = ch_num[zhu];
                        frame_zhuindex[t] = zhu;
                    }
                }
            }
        }
        for(int t = 0; t < frame; t++)
        {
            for(int i = 0; i < state_num[t]; i++)
            {
                oov[t][i] = false;
            }
            
        }
        delta[0][0] = 0;
        phi[0][0] = 0;
        for(int t = 1; t < frame-1; t++)
        {
            for(int j = 0; j < state_num[t]; j++)
            {
                double temp[state_num[t-1]];
                for(int i = 0; i < state_num[t-1]; i++)
                {
                    //string w_now = "";
                    //string w_before = "";
                    char w_now[3], w_before[3];
                    if(frame_zhuindex[t] == -1)
                    {
                        w_now[0] = buffer_seg[n][2*(t-1)];
                        w_now[1] = buffer_seg[n][2*(t-1)+1];
                        w_now[2] = '\0';
                        //w_now.push_back(buffer_seg[n][2*(t-1)+1]);
                        //w_now = {buffer_seg[n][2*(t-1)], buffer_seg[n][2*(t-1)+1], '\0'};
                        if(frame_zhuindex[t-1] == -1)
                        {
                            w_before[0] = buffer_seg[n][2*(t-2)];
                            w_before[1] = buffer_seg[n][2*(t-2)+1];
                            w_before[2] = '\0';
                            //w_before.push_back(buffer_seg[n][2*(t-2)]);
                            //w_before.push_back(buffer_seg[n][2*(t-2)+1]);
                            //w_before = {buffer_seg[n][2*(t-2)], buffer_seg[n][2*(t-2)+1], '\0'};
                        }
                        else if(frame_zhuindex[t-1] >= 0)
                        {
                            w_before[0] = ch[frame_zhuindex[t-1]][2*i];
                            w_before[1] = ch[frame_zhuindex[t-1]][2*i+1];
                            w_before[2] = '\0';
                            //w_before.push_back(ch[frame_zhuindex[t-1]][2*i]);
                            //w_before.push_back(ch[frame_zhuindex[t-1]][2*i+1]);
                            //w_before = {ch[frame_zhuindex[t-1]][2*i], ch[frame_zhuindex[t-1]][2*i+1], '\0'};
                        }
                    }
                    else
                    {
                        w_now[0] = ch[frame_zhuindex[t]][2*j];
                        w_now[1] = ch[frame_zhuindex[t]][2*j+1];
                        w_now[2] = '\0';
                        //w_now.push_back(ch[frame_zhuindex[t]][2*j]);
                        //w_now.push_back(ch[frame_zhuindex[t]][2*j+1]);
                        //w_now = {ch[frame_zhuindex[t]][2*j], ch[frame_zhuindex[t]][2*j+1], '\0'};
                        if(frame_zhuindex[t-1] == -1)
                        {
                            w_before[0] = buffer_seg[n][2*(t-2)];
                            w_before[1] = buffer_seg[n][2*(t-2)+1];
                            w_before[2] = '\0';
                            //w_before.push_back(buffer_seg[n][2*(t-2)]);
                            //w_before.push_back(buffer_seg[n][2*(t-2)+1]);
                            //w_before = {buffer_seg[n][2*(t-2)], buffer_seg[n][2*(t-2)+1], '\0'};
                        }
                        else if(frame_zhuindex[t-1] >= 0)
                        {
                            w_before[0] = ch[frame_zhuindex[t-1]][2*i];
                            w_before[1] = ch[frame_zhuindex[t-1]][2*i+1];
                            w_before[2] = '\0';
                            //w_before.push_back(ch[frame_zhuindex[t-1]][2*i]);
                            //w_before.push_back(ch[frame_zhuindex[t-1]][2*i+1]);
                            //w_before = {ch[frame_zhuindex[t-1]][2*i], ch[frame_zhuindex[t-1]][2*i+1], '\0'};
                        }

                    }
                    if(t == 1)
                    {
                        wid_2 = voc_2.getIndex(w_now);
                        if(wid_2 == Vocab_None)
                        {
                            wid_2 = voc_2.getIndex(Vocab_Unknown);
                            oov[t][j] = true;
                        }
                        wid_1 = voc_2.getIndex("<s>");
                        if(wid_1 == Vocab_None)
                        {
                            wid_1 = voc_2.getIndex(Vocab_Unknown);
                            oov[t-1][i] = true;
                        }
                        VocabIndex context_2[] = {wid_1, Vocab_None};
                        temp[i] = delta[t-1][i] + lm_2.wordProb(wid_2, context_2);
                    }
                    else
                    {
                        wid_2 = voc_2.getIndex(w_now);
                        if(wid_2 == Vocab_None)
                        {
                            wid_2 = voc_2.getIndex(Vocab_Unknown);
                            oov[t][j] = true;
                            
                        }
                        wid_1 = voc_2.getIndex(w_before);
                        if(wid_1 == Vocab_None)
                        {
                            wid_1 = voc_2.getIndex(Vocab_Unknown);
                            oov[t-1][i] = true;
                        }
                        VocabIndex context_2[] = {wid_1, Vocab_None};
                        temp[i] = delta[t-1][i] + lm_2.wordProb(wid_2, context_2);
                    }
                      
                }
                delta[t][j] = max(temp, state_num[t-1]);
                phi[t][j] = argmax(temp, state_num[t-1]);
            }
        }
        double temp[state_num[frame-2]];
        //string w_before = "";
        char w_before[3];
        for(int i = 0; i < state_num[frame-2]; i++)
        {
            if(frame_zhuindex[frame-2] == -1)
            {
                w_before[0] = buffer_seg[n][2*(frame-3)];
                w_before[1] = buffer_seg[n][2*(frame-3)+1];
                w_before[2] = '\0';
                //w_before.push_back(buffer_seg[n][2*(frame-3)]);
                //w_before.push_back(buffer_seg[n][2*(frame-3)+1]);
                //w_before = {buffer_seg[n][2*(frame-3)], buffer_seg[n][2*(frame-3)+1], '\0'};
            }
            else if(frame_zhuindex[frame-2] >= 0)
            {
                w_before[0] = ch[frame_zhuindex[frame-2]][2*i];
                w_before[1] = ch[frame_zhuindex[frame-2]][2*i+1];
                w_before[2] = '\0';
                //w_before.push_back(ch[frame_zhuindex[frame-2]][2*i]);
                //w_before.push_back(ch[frame_zhuindex[frame-2]][2*i+1]);
                //w_before = {ch[frame_zhuindex[frame-2]][2*i], ch[frame_zhuindex[frame-2]][2*i+1], '\0'};
            }
            wid_2 = voc_2.getIndex("</s>");
            if(wid_2 == Vocab_None)
            {
                wid_2 = voc_2.getIndex(Vocab_Unknown);
                oov[frame-1][0] = true;
            }
            wid_1 = voc_2.getIndex(w_before);
            if(wid_1 == Vocab_None)
            {
                wid_1 = voc_2.getIndex(Vocab_Unknown);
                oov[frame-2][i] = true;
            }
            VocabIndex context_2[] = {wid_1, Vocab_None};
            temp[i] = delta[frame-2][i] + lm_2.wordProb(wid_2, context_2);
        }
        delta[frame-1][0] = max(temp, state_num[frame-2]);
        phi[frame-1][0] = argmax(temp, state_num[frame-2]);
        for(int t = frame-2; t >= 1;t--)
        {
            q[t] = phi[t+1][q[t+1]];
            if(frame_zhuindex[t] == -1)
            {
                ans_str[2*(t-1)] = buffer_seg[n][2*(t-1)];
                ans_str[2*(t-1)+1] = buffer_seg[n][2*(t-1)+1];
            }
            else if(frame_zhuindex[t] >= 0)
            {
                ans_str[2*(t-1)] = ch[frame_zhuindex[t]][2*q[t]];
                ans_str[2*(t-1)+1] = ch[frame_zhuindex[t]][2*q[t]+1];
            }
        }
        
        string output = "";
        output.append("<s>");
        for(int t = 1; t < frame - 1; t++)
        {
            
            output.push_back(' ');
            output.push_back(ans_str[2*(t-1)]);
            output.push_back(ans_str[2*(t-1)+1]);
            
        }
        output.append(" </s>\n");
        buffer_out.push_back(output);
        for(int t = 0; t < frame; t++)
        {
            delete [] delta[t];
            delete [] phi[t];
            delete [] oov[t];
        }
        delete [] delta;
        delete [] phi;
        delete [] oov;
        delete [] state_num;
        delete [] frame_zhuindex;
        delete [] q;
    }
    for(int n = 0; n < buffer_out.size(); n++)
    {
        file_out << buffer_out[n];
    }
    for(int i = 0; i < zhu_num; i++)
    {
        delete [] ch[i];
    }
    delete [] ch;
    delete [] ch_num;
    delete [] zhuyin;
    file_out.close(); 
    return 0;
}
