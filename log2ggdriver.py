import os
from google.oauth2.service_account import Credentials
from googleapiclient.discovery import build
from googleapiclient.http import MediaFileUpload
import mimetypes
import pandas as pd 
from datetime import datetime
import os



KEY_FILE_PATH = '/home/ad/Downloads/earnest-sight-435100-u4-c2e6626891e0.json'
SCOPES = ['https://www.googleapis.com/auth/drive.file']
LOCAL_FILE_PATH = '/home/ad/Documents/PracticeCpp/av.txt'
DRIVE_FOLDER_ID = "16xkA8LlK7wEYnw0SsJCQkH-dF7v2wmps" 

def extract_last_block_to_df(input_csv):

    with open(input_csv, 'r', encoding='utf-8') as f:
        lines = [line.strip() for line in f if line.strip()]

    start_idx = None
    end_idx = None
    for i in range(len(lines)-1, -1, -1):
        if lines[i].startswith("Xe_ID,"):
            start_idx = i
            break
    for i in range(start_idx, len(lines)):
        if lines[i].startswith("totalcar,"):
            end_idx = i+2  
            break

    if start_idx is not None and end_idx is not None:
        block = lines[start_idx:end_idx]
        header = block[0].split(',')
        data_rows = [row.split(',') for row in block[1:-2]]
        df = pd.DataFrame(data_rows, columns=header)

        for col in ["Thoi_gian_chay", "Do_lech", "Gia_tri_ham_muc_tieu"]:
            if col in df.columns:
                df[col] = pd.to_numeric(df[col], errors='coerce')

        total_header = block[-2].split(',')
        total_data = block[-1].split(',')
        df_global = pd.DataFrame([total_data], columns=total_header)
        df_global["map"] = "map"
        df_global["algorithm"] = "dijkstra"
        df_global["version"] = "dijkstra"
        df_global["num_fail"] = (df["reachTarget"] == "false").sum()
        df_global["sum_objective"] = df["Gia_tri_ham_muc_tieu"][df["reachTarget"] == "true"].sum()
        df_global["sum_time"] = df["Thoi_gian_chay"][df["reachTarget"] == "true"].sum()
        df_global["sum_deviation"] = df["Do_lech"][df["reachTarget"] == "true"].sum()
        df_global["local_time"] = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")

        if "reachTarget" in df.columns:
            df_new = df.drop(columns=["reachTarget"])
        else:
            df_new = df.copy()
        df_new["local_time"] = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        concat_csv(df_new, filename="car_log.csv")
        concat_csv(df_global, filename="global_log.csv")
        return "car_log.csv", "global_log.csv"
    else:
        print("Không tìm thấy block hợp lệ.")
        return None, None
    
def concat_csv(df, filename):
    if os.path.exists(filename):
        df_old = pd.read_csv(filename)
        df_all = pd.concat([df_old, df], ignore_index=True)
    else:
        df_all = df
    df_all.to_csv(filename, index=False)

def upload_file_to_drive(local_file_path, file_name_on_drive, drive_folder_id=None):
    from googleapiclient.errors import HttpError

    creds = Credentials.from_service_account_file(KEY_FILE_PATH, scopes=SCOPES)
    service = build('drive', 'v3', credentials=creds)

    # Tìm file trùng tên trong folder (nếu có)
    query = f"name='{file_name_on_drive}'"
    if drive_folder_id:
        query += f" and '{drive_folder_id}' in parents"
    results = service.files().list(q=query, fields="files(id, name)").execute()
    files = results.get('files', [])

    mimetype, _ = mimetypes.guess_type(local_file_path)
    if mimetype is None:
        mimetype = 'application/octet-stream'
    media = MediaFileUpload(local_file_path, mimetype=mimetype, resumable=True)

    if files:
        # Đã có file, update
        file_id = files[0]['id']
        file = service.files().update(
            fileId=file_id,
            media_body=media
        ).execute()
        print(f"Đã ghi đè file: {file_name_on_drive}")
        return file_id
    else:
        # Chưa có, tạo mới
        file_metadata = {'name': file_name_on_drive}
        if drive_folder_id:
            file_metadata['parents'] = [drive_folder_id]
        file = service.files().create(
            body=file_metadata,
            media_body=media,
            fields='id, name, webViewLink'
        ).execute()
        print(f"Đã tạo file mới: {file_name_on_drive}")
        return file.get('id')

if __name__ == '__main__':
    file1, file2 = extract_last_block_to_df("/home/ad/omnetpp-6.1/samples/week6/simulations/veins/vehicle.csv")
    upload_file_to_drive(file1, file1, drive_folder_id=DRIVE_FOLDER_ID)
    upload_file_to_drive(file2, file2, drive_folder_id=DRIVE_FOLDER_ID)