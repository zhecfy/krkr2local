#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

const char CR = 0x0D;
const char LF = 0x0A;
const char SPACE = 0x20;

const int MAX_FILE_LENGTH = 1000000;
char file1[MAX_FILE_LENGTH], file2[MAX_FILE_LENGTH];
bool mark[MAX_FILE_LENGTH];

void print_help()
{
	cout << "usage: krkr2local [options] [files]" << endl;
	cout << endl;
	cout << "options:" << endl;
	cout << "-h or -help" << "\n\t" << "Print help." << endl;
	cout << "-ph or -printhex [file]" << "\n\t" << "Print target file in hex." << endl;
	cout << "-dr or -deleteruby [input] [output]" << "\n\t" << "Delete ruby texts in [input]'s scripts." << endl;
	cout << "-ef or -eraseformat [input] [output]" << "\n\t" << "Erase all formatting marks in [input], generate [output] containing only plain text." << endl;
	cout << "-ds or -deletespace [input] [output]" << "\n\t" << "Delete spaces and empty lines in [input]." << endl; 
	cout << "-r or -replace [format] [newtext] [output]" << "\n\t" << "Replace all plain text in [format] with the corresponding text in [newtext]." << endl;
	return;
}

void print_char_in_hex(char c)
{
	int u = (unsigned char)c;
	int first = u / 16,second = u % 16;
	if (first < 10) cout << first;
	else cout << (char)('A' + first - 10);
	if (second < 10) cout << second;
	else cout << (char)('A' + second - 10);
	return;
}

void print_file_in_hex(const char file_name[])
{
	FILE *fp = fopen(file_name,"rb");
	if (fp == NULL) {cout << "Unable to open "<< file_name << endl; return;}
	else cout << file_name << " opened successfully." << endl;
	
	char row[16];
	int cnt = 0;
	while (fread(row,1,16,fp) == 16)
	{
		cnt++;
		for (int i = 0;i < 16;i++)
		{
			print_char_in_hex(row[i]);
			putchar(' ');
		}
		for (int i = 0;i < 16;i++)
		{
			if (row[i] != '\r' && row[i] != '\n' && row[i] >= 0x20 && row[i] <= 0x7E) printf("%c  ",row[i]);
			else if (row[i] == '\r') printf("\\r ");
			else if (row[i] == '\n') printf("\\n ");
			else if (row[i] < 0x20 || row[i] > 0x7E) printf("?? ");
			else return;
		}
		cout << endl;
	}
	cout << "total char: " << cnt * 16 << endl;
	fclose(fp);
	return;
}

void delete_ruby(const char jp_file_name[], const char jp_noruby_file_name[], char jpfile[], bool deleted[])
{
	const char ruby[11] = "ruby text=";
	FILE *jp_fp = fopen(jp_file_name,"rb");
	if (jp_fp == NULL) {cout << "Unable to open "<< jp_file_name << endl; return;}
	else cout << jp_file_name << " opened successfully." << endl;
	FILE *jp_noruby_fp = fopen(jp_noruby_file_name,"wb");
	if (jp_noruby_fp == NULL) {cout << "Unable to open " << jp_noruby_file_name << endl; return;}
	else cout << jp_noruby_file_name << " opened successfully." << endl;
	
	int cnt = 0;
	memset(jpfile,0,sizeof(*jpfile));
	memset(deleted,0,sizeof(*deleted));
	while (fread(&jpfile[cnt],1,1,jp_fp) == 1) cnt++;
	cout << "Read complete, total text = " << cnt << endl;
	int ruby_cnt = 0;
	for (int i = 0;i < cnt;i++)
	{
		bool flag = true;
		for (int j = 0;j < 10;j++)
		{
			if ((int)jpfile[i + j] != (int)ruby[j])
			{
				flag = false;
				break;
			}
		}
		if (flag)
		{
			ruby_cnt++;
			int del_l = i,del_r = i;
			while (jpfile[del_l] != '[') del_l--;
			while (jpfile[del_r] != ']') del_r++;
			for (int k = del_l;k <= del_r;k++)
			{
				deleted[k] = true;
			}
		}
	}
	cout << "Ruby text count = " << ruby_cnt << endl;
	int noruby_cnt = 0;
	for (int i = 0;i < cnt;i++)
	{
		if (!deleted[i])
		{
			fputc(jpfile[i],jp_noruby_fp);
			noruby_cnt++;
		}
	}
	cout << "Total noruby text = " << noruby_cnt << endl;
	fclose(jp_fp);
	fclose(jp_noruby_fp);
	return;
}

void erase_format(const char jp_file_name[], const char jp_plain_file_name[], char jpfile[])
{
	FILE *jp_fp = fopen(jp_file_name,"rb");
	if (jp_fp == NULL) {cout << "Unable to open " << jp_file_name << endl; return;}
	else cout << jp_file_name << " opened successfully." << endl;
	FILE *jp_plain_fp = fopen(jp_plain_file_name,"wb");
	if (jp_plain_fp == NULL) {cout << "Unable to open " << jp_plain_file_name << endl; return;}
	else cout << jp_plain_file_name << " opened successfully." << endl;
	
	int cnt = 0;
	int plain_cnt = 0;
	memset(jpfile,0,sizeof(*jpfile));
	while (fread(&jpfile[cnt],1,1,jp_fp) == 1) cnt++;
	cout << "Read complete, total formatted text = " << cnt << endl;
	bool in_plain = false;
	for (int i = 0;i < cnt;)
	{
		if (jpfile[i] == '[')
		{
			if (in_plain)
			{
				in_plain = false;
				fputc(CR,jp_plain_fp);
				fputc(LF,jp_plain_fp);
			}
			while (jpfile[i] != ']') i++;
			i++;
		}
		else if (jpfile[i] == ';' || jpfile[i] == '*')
		{
			if (in_plain)
			{
				in_plain = false;
				fputc(CR,jp_plain_fp);
				fputc(LF,jp_plain_fp);
			}
			while (jpfile[i] != '\r') i++;
		}
		else if (jpfile[i] == '\r' || jpfile[i] == '\n')
		{
			if (in_plain)
			{
				in_plain = false;
				fputc(CR,jp_plain_fp);
				fputc(LF,jp_plain_fp);
			}
			i++;
		}
		else 
		{
			in_plain = true;
			plain_cnt++;
			fputc(jpfile[i],jp_plain_fp);
			i++;
		}
	}
	cout << "Total plain text = " << plain_cnt << endl;
	fclose(jp_fp);
	fclose(jp_plain_fp);
	return;
}

void delete_space(const char cn_file_name[], const char cn_nospace_file_name[], char cnfile[])
{
	FILE *cn_fp = fopen(cn_file_name,"rb");
	if (cn_fp == NULL) {cout << "Unable to open " << cn_file_name << endl; return;}
	else cout << cn_file_name << " opened successfully." << endl;
	FILE *cn_nospace_fp =fopen(cn_nospace_file_name,"wb");
	if (cn_nospace_fp == NULL) {cout << "Unable to open " << cn_nospace_file_name << endl; return;}
	else cout << cn_nospace_file_name << " opened successfully." << endl;
	
	int cnt = 0, nospace_cnt = 0;
	while (fread(&cnfile[cnt],1,1,cn_fp) == 1) cnt++;
	cout << "Read complete, total cn text = " << cnt << endl;
	for (int i = 0;i < cnt;i++)
	{
		if (cnfile[i] == SPACE)
		{
			continue;
		}
		if (cnfile[i] == CR || cnfile[i] == LF)
		{
			int j = i;
			bool allCRLFSPACE = true;
			while (j >= 0)
			{
				j--;
				if (cnfile[j] == cnfile[i]) break;
				if (cnfile[j] == CR || cnfile[j] == LF || cnfile[j] == SPACE) continue;
				else
				{
					allCRLFSPACE = false;
					break;
				}
			}
			if (allCRLFSPACE) continue;
			else 
			{
				nospace_cnt++;
				fputc(cnfile[i],cn_nospace_fp);
				continue;
			}
		}
		else 
		{
			nospace_cnt++;
			fputc(cnfile[i],cn_nospace_fp);
			continue;
		}
	}
	cout << "Total nospace text = " << nospace_cnt << endl;
	
	
	fclose(cn_fp);
	fclose(cn_nospace_fp);
}

void replace(const char jp_file_name[], const char cn_plain_file_name[], const char cn_file_name[], char jpfile[], char cnfile[])
{
	FILE *jp_fp,*cn_plain_fp,*cn_fp;
	jp_fp = fopen(jp_file_name,"rb");
	if (jp_fp == NULL) cout << "Unable to open " << jp_file_name << endl;
	else cout << jp_file_name << " opened successfully." << endl;
	cn_plain_fp = fopen(cn_plain_file_name,"rb");
	if (cn_plain_fp == NULL) cout << "Unable to open " << cn_plain_file_name << endl;
	else cout << cn_plain_file_name << " opened successfully." << endl;
	cn_fp = fopen(cn_file_name,"wb");
	if (cn_fp == NULL) cout << "Unable to open " << cn_file_name << endl;
	else cout << cn_file_name << " opened successfully." << endl;
	
	int jp_cnt = 0;
	int cn_cnt = 0;
	while (fread(&jpfile[jp_cnt],1,1,jp_fp) == 1) jp_cnt++;
	cout << "Read complete, total jp text = " << jp_cnt << endl;
	while (fread(&cnfile[cn_cnt],1,1,cn_plain_fp) == 1) cn_cnt++;
	cout << "Read complete, total cn text = " << cn_cnt << endl;
	int j = 0;
	int cn_fp_cnt = 0;
	for (int i = 0;i < jp_cnt;)
	{
		if (jpfile[i] == '[')
		{
			while (jpfile[i] != ']')
			{
				fputc(jpfile[i],cn_fp);
				cn_fp_cnt++;
				i++;
			}
			fputc(jpfile[i],cn_fp);
			cn_fp_cnt++;
			i++;
		}
		else if (jpfile[i] == ';' || jpfile[i] == '*')
		{
			while (jpfile[i] != '\r')
			{
				fputc(jpfile[i],cn_fp);
				cn_fp_cnt++;
				i++;
			}
		}
		else if (jpfile[i] == '\r' || jpfile[i] == '\n')
		{
			fputc(jpfile[i],cn_fp);
			cn_fp_cnt++;
			i++;
		}
		else
		{
			while (jpfile[i] != '[' && jpfile[i] != ';' && jpfile[i] != '*' && jpfile[i] != '\r' && jpfile[i] != '\n')
			{
				i++;
			}
			while (cnfile[j] != '\r' && cnfile[j] != '\n')
			{
				fputc(cnfile[j],cn_fp);
				cn_fp_cnt++;
				j++;
			}
			j++; //CR
			j++; //LF
		}
	}
	cout << "Total cn text in format = " << cn_fp_cnt << endl;
	
	fclose(jp_fp);
	fclose(cn_plain_fp);
	fclose(cn_fp);
}

int main(int argc, char* argv[])
{
	//system("chcp 65001");
	if (argc <= 1) print_help();
	else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-help") == 0) print_help();
	else if (strcmp(argv[1], "-ph") == 0 || strcmp(argv[1], "-printhex") == 0)
	{
		if (argc != 3) print_help();
		else
		{
			const char* file = argv[2];
			print_file_in_hex(file);
		}
	}
	else if (strcmp(argv[1], "-dr") == 0 || strcmp(argv[1], "-deleteruby") == 0)
	{
		if (argc != 4) print_help();
		else
		{
			const char *input = argv[2], *output = argv[3];
			delete_ruby(input,output,file1,mark);
		}
	}
	else if (strcmp(argv[1], "-ef") == 0 || strcmp(argv[1], "-eraseformat") == 0)
	{
		if (argc != 4) print_help();
		else
		{
			const char *input = argv[2], *output = argv[3];
			erase_format(input,output,file1);
		}
	}
	else if (strcmp(argv[1], "-ds") == 0 || strcmp(argv[1], "-deletespace") == 0)
	{
		if (argc != 4) print_help();
		else
		{
			const char *input = argv[2], *output = argv[3];
			delete_space(input,output,file1);
		}
	}
	else if (strcmp(argv[1], "-r") == 0 || strcmp(argv[1], "-replace") == 0)
	{
		if (argc != 5) print_help();
		else
		{
			const char *format = argv[2], *new_text = argv[3], *output = argv[4];
			replace(format,new_text,output,file1,file2);
		}
	}
	return 0;
}

