from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from supabase import create_client, Client
import math, os
from dotenv import load_dotenv

load_dotenv()

SUPABASE_URL = os.getenv("SUPABASE_URL")
SUPABASE_KEY = os.getenv("SUPABASE_KEY")

supabase: Client = create_client(SUPABASE_URL, SUPABASE_KEY)

app = FastAPI(title="Patient Monitoring API...")

class PatientData(BaseModel):
    name: str
    age: int
    sex: str
    blood_group: str
    height: float
    weight: float
    known_conditions: str | None = None
    temperature: float
    heart_rate: float
    spo2: float
    ecg_data: list[dict]
    
# helper
def calculate_bmi(height_cm: float, weight_kg: float):
    h_m = height_cm/100
    return round(weight_kg / (h_m**2), 2)

# routes
@app.get('/')
def index():
    return {"title": "welcome to IOT API", "created by": "Anurag Panda"}

@app.post("/upload-patient-data")
def upload_patient_data(p: PatientData):
    bmi = calculate_bmi(p.height, p.weight)
    
    data ={
        "name": p.name,
        "age": p.age,
        "sex": p.sex,
        "blood_group": p.blood_group,
        "height": p.height,
        "weight": p.weight,
        "known_conditions": p.known_conditions,
        "bmi": bmi,
        "temperature": p.temperature,
        "heart_rate": p.heart_rate,
        "spo2": p.spo2,
        "ecg_data": p.ecg_data
    }
    
    res = supabase.table("patients").insert(data).execute()
    
    if res.data:
        return {"status": "success", "patient": res.data[0]}
    raise HTTPException(status_code=400, detail="Failed to register patient")

